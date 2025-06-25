/**
 * Openedges Enlight NPU Test application
 *
 * NPU Verification register test application
*/
#include <stdio.h>
#include "npu_reg.h"
#include "test_npu.h"
#include "npu_api.h"
#include "chip_test.h"

#include "sys/time.h"

static unsigned int npu_num_core = 0;
static unsigned int npu_major_ver = 0;
static unsigned int npu_minor_ver = 0;
static unsigned int npu_ecc = 0;
static unsigned int npu_nd_cs = 0;
static unsigned int npu_product_id = 0;
static unsigned int hci_base = 0;

static int wdt_test(npu_t* npu);
static int activate_wdt(
    npu_t*      npu,
    uint32_t    ewdt_time_msec,
    uint32_t    cpu_freq_mhz,
    int         iwdt_margin,
    int         wdt_en);
static void deactivate_wdt(npu_t* npu);
static int polling_wdt_irq(
    npu_t*      npu,
    uint32_t    ewdt_time_msec,
    uint32_t    host_cpu_freq_mhz);
static int verify_hci_reg(npu_t* npu, uint32_t ref);

int npu_wdt_test_main(npu_t* npu)
{
    int status = 0;
    int ret;

    unsigned int val;

    npu_read_reg(npu, ADDR_NPU_ID_CODE, &val);

    npu_num_core   = (val >> 24) & 0xF;
    npu_major_ver  = (val >> 20) & 0xF;
    npu_minor_ver  = (val >> 18) & 0x3;
    npu_product_id = (val >>  0) & 0xFFFF;
    npu_ecc        = (val >> 16) & 0x1;
    npu_num_core   += 1;

    if (((val >> 16) & 0xFF) >= 0x21)
        npu_nd_cs = 1;
    else
        npu_nd_cs = 0;

    hci_base = ADDR_NPU_MLX_C0_HCI_00;

    if ((npu_product_id & 0xFFFF) != 0xED9E)
    {
        npu_test_printf("Unknown NPU prodcut ID\n");
        npu_test_printf("fail\n");
        npu_test_printf("========================\n\n");
        return 1;
    }

    npu_test_printf("========================\n");
    npu_test_printf("NPU product id: 0x%04X\n", npu_product_id);
    npu_test_printf("NPU major ver : 0x%X\n", npu_major_ver);
    npu_test_printf("NPU minor ver : 0x%X\n", npu_minor_ver);
    npu_test_printf("NPU ecc       : 0x%X\n", npu_ecc);
    npu_test_printf("NPU core num  : 0x%X\n", npu_num_core);
    npu_test_printf("========================\n");

    do {
        if (!npu_nd_cs) {
            npu_test_printf("WDT not supported in 0x%08X\n", npu_product_id);
            status = -1;
            break;
        }

        npu_test_printf("========================\n");
        npu_test_printf("WDT Test Start\n");
        npu_test_printf("========================\n");

        status = wdt_test(npu);
        if (status)
            break;

    } while(0);

    if (status)
        npu_test_printf("fail\n");
    else
        npu_test_printf("success\n");

    npu_test_printf("========================\n");
    npu_test_printf("TEST_INFO: DONE         \n");
    npu_test_printf("========================\n");

    return status;
}

static int wdt_test(npu_t* npu)
{
    unsigned int val;

    uint32_t ewdt_time_msec;
    uint32_t cpu_freq_mhz;
    uint32_t host_cpu_freq_mhz;

    int iwdt_margin;    // default is 200
    int wdt_en;
    int poll;           // 0: Timeout, 1: IRQ are set

    int wdt_set = 0;    // 1: set , -1: check timer arguments
    int err = 0;
    int ret;

    const int debug = 0;

    ret = npu_reset(npu, 0, NULL, NULL);
    if (ret) {
        npu_test_printf("%d: npu reset() fail\n", ret);
        return ret;
    }

    //clear terminate signal
    for(int i = 0; i < npu_num_core; i++){
        uint32_t base;
        uint32_t data;
        base = hci_base + 0x10 + (i * 0x40);
        data = 0x0;
        npu_write_reg(npu, base, data);
        if (debug)
            npu_test_printf("[WR] 0x%08x <= 0x%08x\r\n", base, data);
    }

    // WDT setting
    ewdt_time_msec = 1000;
    npu_test_printf("Watchdog timer setting: %d msec\n\n", ewdt_time_msec);
    //cpu_freq_mhz = 30;
    //host_cpu_freq_mhz = 45;
    cpu_freq_mhz = 800;
    host_cpu_freq_mhz = 1000;

    // ----------
    // Test 1
    //   : do not activate wdt with re-arm configuration
    // ----------
    npu_test_printf("WDT test 1: Watchdog-off test\n");
    iwdt_margin = -200;
    wdt_en = 0;

    wdt_set = activate_wdt(npu, ewdt_time_msec, cpu_freq_mhz, iwdt_margin, wdt_en);
    if (wdt_set == 1) {
        // Test results
        poll = polling_wdt_irq(npu, ewdt_time_msec, host_cpu_freq_mhz);
        if (poll == 0) {
            npu_test_printf("PASS: WDT test 1\n");
        }
        else {
            npu_test_printf("   WDT interupt occurs\n");
            npu_test_printf("FAIL: WDT test 1\n");
            return -1;
        }

        // IRQ check
        err = err + verify_hci_reg(npu, 0x100);
    } else {
        err = err - 1;
    }
    npu_test_printf("\n");


    // ----------
    // Test 2
    //   : activate wdt(re-arm success)
    // ----------
    npu_test_printf("WDT test 2: Watchdog-on test\n");
    iwdt_margin = 200;  // default is 200
    wdt_en = 1;
    wdt_set = activate_wdt(npu, ewdt_time_msec, cpu_freq_mhz, iwdt_margin, wdt_en);

    if (wdt_set == 1) {
        // Test results
        poll = polling_wdt_irq(npu, ewdt_time_msec, host_cpu_freq_mhz);
        if (poll == 0) {
            npu_test_printf("PASS: WDT test 2\n");
        }
        else {
            npu_test_printf("   WDT interupt occurs\n");
            npu_test_printf("FAIL: WDT test 2\n");
            return -1;
        }

        err = err + verify_hci_reg(npu, 0x1000100);
    } else {
        err = err - 1;
    }

    npu_test_printf("\n");

    // ----------
    // Test 3
    //   : activate wdt (re-arm fail)
    // ----------
    npu_test_printf("WDT test 3: Watchdog interupt invoke test(re-arm fail)\n");
    iwdt_margin = -200;
    wdt_en = 1;
    wdt_set = activate_wdt(npu, ewdt_time_msec, cpu_freq_mhz, iwdt_margin, wdt_en);

    if (wdt_set == 1) {
        poll = polling_wdt_irq(npu, ewdt_time_msec, host_cpu_freq_mhz);
        // Test results
        if (poll == 1) {
            npu_test_printf("PASS: WDT test 3\n");
        }
        else {
            npu_test_printf("   No WDT interupt occurs\n");
            npu_test_printf("FAIL: WDT test 3\n");
            return -1;
        }

        // IRQ check
        err = err + verify_hci_reg(npu, 0x3000100);
    } else {
        err = err - 1;
    }

    //npu_ecc_t ecc_para;
    //npu_chiptest_t ct_para;

    //ecc_para.disable_ue_fail = 0x0;
    //ecc_para.disable_ce_fail = 0x0;
    //ecc_para.disable_wdt = 0;

    //ct_para.dev_mlx_bin_idx = 0;           /**<  CHIPTEST dbg only: MLX test bin idx 1~8   */   
    //ct_para.dev_wdt_ext_cnt = 0;
    //ct_para.dev_wdt_int_cnt = 0;
    //ct_para.dev_ecc_test_ctrl = 0;
    //ct_para.dev_mlx_err_inj_mask_data = 0;
    //ct_para.dev_mlx_err_inj_mask_par = 0;

    ret = npu_reset(npu, 0, NULL, NULL);
    if (ret) {
        npu_test_printf("%d: npu reset() fail\n", ret);
        return ret;
    }

    npu_test_printf("\n");

    // ----------
    // Test 4
    //   : activate wdt (re-arm) -> de-activate wdt
    // ----------
    // activate wdt
    npu_test_printf("WDT test 4: npu reset & activate wdt\n");

    iwdt_margin = 200;  // default is 200
    wdt_en = 1;

    wdt_set = activate_wdt(npu, ewdt_time_msec, cpu_freq_mhz, iwdt_margin, wdt_en);

    if (wdt_set == 1) {
        poll = polling_wdt_irq(npu, ewdt_time_msec, host_cpu_freq_mhz);
        if (poll == 1) {
            npu_test_printf("   WDT interupt occurs\n");
            npu_test_printf("FAIL: WDT test 4\n");
            return -1;
        }
        err = err + verify_hci_reg(npu, 0x1000100);

        // deactivate wdt
        npu_test_printf("deactivate wdt\n");
        deactivate_wdt(npu);

        if (poll == 0)
            npu_test_printf("PASS: WDT test 4\n");
        poll = polling_wdt_irq(npu, ewdt_time_msec, host_cpu_freq_mhz);
        if (poll == 1) {
            npu_test_printf("   WDT interupt occurs\n");
            npu_test_printf("FAIL: WDT test 4\n");
            return -1;
        }

        err = err + verify_hci_reg(npu, 0x0000100);
    } else {
        err = err - 1;
    }

    return err;
}

static int activate_wdt(
    npu_t*      npu,
    uint32_t    ewdt_time_msec,
    uint32_t    cpu_freq_mhz,
    int         iwdt_margin,
    int         wdt_en)
{
    /* Set HCI register to activate watchdog timer 
     *
     * Input
     *   ewdt_time_msec  : external watchdog timer time (milisecond)
     *   cpu_freq_mhz    : cpu clock frequency (MHz)
     *   iwdt_margin     : internal timer margin (1/cpu_freq)
     *                     when margin > 175 cpu cycles, external watchdog does not expire (re-arm)
     *   wdt_en          : watchdog enable
     */

    int exception = 0;
    int ret;  // -1: setting error, 0: no error
    uint32_t ext_wdt_cnt, int_wdt_cnt;
    uint32_t ext_msec_max;

    const int debug = 0;

    ext_wdt_cnt = ewdt_time_msec * cpu_freq_mhz * 1000;
    int_wdt_cnt = (uint32_t)((ext_wdt_cnt - iwdt_margin) / 100);
    npu_test_printf("ext_wdt_cnt: 0x%08X, %12d\n", ext_wdt_cnt, ext_wdt_cnt);
    npu_test_printf("int_wdt_cnt: 0x%08X, %12d\n", int_wdt_cnt, int_wdt_cnt);

    // ext_msec_max is maximum value to set millisecond
    ext_msec_max = (uint32_t)(0xFFFFFFFFU / 1000000);
    ext_msec_max = (uint32_t)(ext_msec_max / cpu_freq_mhz) * 1000; 

    if (ewdt_time_msec > ext_msec_max)
        exception = 1;
    if (int_wdt_cnt == 0)
        exception = 1;

    if (exception == 0) {
        // HCI setting
        for (int i=0; i < npu_num_core; i++){
            uint32_t base;
            uint32_t data;
            volatile int timeout_cnt = 100;
            
            base = hci_base + 0x04 + (i * 0x40);
            data = 0xCD09;
            npu_write_reg(npu, base, data);
            if (debug) {
                npu_test_printf("[WR] 0x%08x <= 0x%08x\r\n", base, data);

                npu_read_reg(npu, base, &data);
                npu_test_printf("[RD] 0x%08x <= 0x%08x\r\n", base, data);
            }

            base = hci_base + 0x08 + (i * 0x40);
            data = ext_wdt_cnt;
            npu_write_reg(npu, base, data);
            if (debug)
                npu_test_printf("[WR] 0x%08x <= 0x%08x\r\n", base, data);

            base = hci_base + 0x0C + (i * 0x40);
            data = int_wdt_cnt;
            npu_write_reg(npu, base, data);
            if (debug)
                npu_test_printf("[WR] 0x%08x <= 0x%08x\r\n", base, data);

            if (wdt_en) {
                base = hci_base + (i * 0x40);
                data = 0x1000101;  // [24] WDTEN, [8] INTDIS, [0] run
                npu_write_reg(npu, base, data);

                npu_test_printf("Core %d: watchdog timer is activated\n", i);

            } else {
                base = hci_base + (i * 0x40);
                data = 0x0000101;  // [24] WDTEN, [8] INTDIS, [0] run
                npu_write_reg(npu, base, data);

                npu_test_printf("Core %d: Watchdog timer is not activated\n", i);
            }

            do {
                npu_read_reg(npu, base, &data);

                if (!(data & 16))
                    break;

                sleep(.1);

                if (--timeout_cnt <= 0) {
                    npu_test_printf("MLX cmd time out!\n");
                    break;
                }

            } while(1);
        }

        ret = 1;
    } else {
        npu_test_printf("WARN: Watchdog timer is not activated\n");
        npu_test_printf("FAIL: MLX does not run\n");
        npu_test_printf("Check watchdog timer setting\n");

        ret = -1;
    }

    return ret;
}

static void deactivate_wdt(npu_t* npu)
{
    for (int i=0; i < npu_num_core; i++) {
        npu_write_reg(npu, hci_base + 0x04 + (i * 0x40), 0xCD09);
        npu_write_reg(npu, hci_base + 0x08 + (i * 0x40), 0xFFFFFFFF);
        npu_write_reg(npu, hci_base + 0x0C + (i * 0x40), 0x0);
        npu_write_reg(npu, hci_base + (i * 0x40), 0x0000101);  // [24] WDTEN,[8] INTDIS, [0] run
    }
}

static int verify_hci_reg(npu_t* npu, uint32_t ref)
{
    uint32_t val;
    for (int i = 0; i < npu_num_core; i = i + 1) {
        npu_read_reg(npu, hci_base + (i * 0x40), &val);
        if (val != ref) {
            npu_test_printf("FAIL: Core %d HCI_CTRL: HW(0x%X), ref(0x%x)\n", i, val, ref);
            return -1;
        }
    }
    return 0;
}


static int get_us_time()
{
    long int ret;
    struct timeval c_time;

    gettimeofday(&c_time, NULL);

    ret = c_time.tv_sec * 1000000;
    ret += c_time.tv_usec;

    //printf("%ld :%s\n", ret, __func__);

    return ret;
}

static int polling_wdt_irq(
    npu_t* npu,
    uint32_t ewdt_time_msec,
    uint32_t host_cpu_freq_mhz)
{
    /* Polling WDTINT(HCI CTRL register) during 3*ewdt_time_msec
     *
     * Output
     *   0 : Timeout
     *   1 : IRQ are set
     */
    int ret = 0;
    volatile int wdt_irq = 0;
    uint32_t start_cycle;
    uint32_t diff_cycle;
    uint32_t timeout_cycle;
    uint32_t polling_time;
    uint32_t ntimes;

    diff_cycle = 0;
    start_cycle = get_us_time();
    ntimes = 3;
    timeout_cycle = ewdt_time_msec * 1000 * ntimes;

    npu_test_printf("   wait WDTINT for %d(msec)\n", timeout_cycle/1000);

    do {
        wdt_irq = 0;

        for (int i = 0; i < npu_num_core; i++) {
            uint32_t addr = (uint32_t)hci_base + (i * 0x40);
            uint32_t data;
            npu_read_reg(npu, addr, &data);
            wdt_irq += (data >> 25) & 0x1; // [25] WDTINT
        }

        diff_cycle = get_us_time() - start_cycle;
        if (diff_cycle > timeout_cycle) {
            npu_test_printf("   wait done\n");
            ret = 0;
            break;
        }

        if (wdt_irq == npu_num_core) {
            npu_test_printf("   wdt_irq asserted\n");
            ret = 1;
            break;
        }

    } while(1);

    return ret;
}
