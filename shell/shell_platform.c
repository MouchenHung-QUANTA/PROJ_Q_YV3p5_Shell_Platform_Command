/*
    NAME: PLATFORM COMMAND
    FILE: shell_platform.c
    DESCRIPTION: OEM commands including gpio, i2c_slave relative function access.
    AUTHOR: MouchenHung
    DATE/VERSION: 2022.01.11 - v1.0.0
    Note: 
    (1) User command table 
          [topic]               [description]				[support] 	[command]
        * GPIO												O
            * List group		List gpios in group			o			platform gpio list_group <gpio_device>
            * List all			List all gpios				o			platform gpio list_all
            * Get				Get one gpio				o			platform gpio get <gpio_num>
            * Set value			Set one gpio value			o			platform gpio set val <gpio_num> <value>
            * Set direction		Set one gpio direction		x			platform gpio set dir <gpio_num> <value>
        * I2C SLAVE											X

    (2) Some hard code features need to be modified if CHIP is different

    (3) GPIO List all/Get/Set value/Set direction are not protected by PINMASK_RESERVE_CHECK. It might cause system-hang problem!
*/

#include <zephyr.h>
#include <sys/printk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <logging/log.h>
#include <shell/shell.h>
#include <device.h>
#include <devicetree.h>

/* Include GPIO */
#include <drivers/gpio.h>
#include "plat_gpio.h"
#include "hal_gpio.h"

/* Log */
LOG_MODULE_REGISTER(mc_shell_platform);

/* Code info */
#define RELEASE_VERSION "v1.0.0"
#define RELEASE_DATE "2022.01.11"

/* HARDCODE - GPIO */
#define PINMASK_RESERVE_CHECK 1
#define GPIO_DEVICE_PREFIX "GPIO0_"
#define GPIO_RESERVE_PREFIX "Reserve"
#define NUM_OF_GROUP 6
#define NUM_OF_GPIO_IS_DEFINE 167

int num_of_pin_in_one_group_lst[NUM_OF_GROUP] = {32, 32, 32, 32, 32, 16};
char GPIO_GROUP_NAME_LST[NUM_OF_GROUP][10] = {"GPIO0_A_D", "GPIO0_E_H", "GPIO0_I_L", "GPIO0_M_P", "GPIO0_Q_T", "GPIO0_U_V"};

/* HARDCODE - I2C SLAVE */
//TODO

/* others */
enum GPIO_ACCESS {
    GPIO_READ,
    GPIO_WRITE
};

/*********************************************************************************************************
 * TOOL SECTION
**********************************************************************************************************/
#if PINMASK_RESERVE_CHECK
enum CHECK_RESV {
    CHECK_BY_GROUP_IDX,
    CHECK_BY_GLOBAL_IDX
};
static int gpio_check_reserve(const struct device *dev, int gpio_idx, enum CHECK_RESV mode) 
{
    if (!dev)
        return 1;
    
    const struct gpio_driver_config *const cfg = (const struct gpio_driver_config *)dev->config;

    if (!cfg->port_pin_mask)
        return 1;

    int gpio_idx_in_group;
    if (mode == CHECK_BY_GROUP_IDX) {
        gpio_idx_in_group = gpio_idx;
    } else if (mode == CHECK_BY_GLOBAL_IDX) {
        gpio_idx_in_group = gpio_idx%GPIO_GROUP_SIZE;
    } else
        return 1;

    if ( (cfg->port_pin_mask & (gpio_port_pins_t)BIT(gpio_idx_in_group) ) == 0U)
        return 1;

    return 0;
}
#endif

static int gpio_access_cfg(const struct shell *shell, int gpio_idx, enum GPIO_ACCESS mode, int *data)
{
    if (!shell)
        return 1;

    if (gpio_idx >= GPIO_CFG_SIZE || gpio_idx < 0)
        return 1;

    /* check whether reserve pin defined in pin NAME */
    if ( !strncmp(GPIO_RESERVE_PREFIX, gpio_name[gpio_idx], 7) )
        return 1;

    char *pin_dir = (gpio_cfg[gpio_idx].direction == GPIO_INPUT) ? "input":"output";
    int val = gpio_get(gpio_idx);

    switch (mode)
    {
    case GPIO_READ:
        if (val == 0 || val == 1)
            shell_print(shell, "[%-3d] %-35s: %-6s | %d", gpio_idx, gpio_name[gpio_idx], pin_dir, val);
        else
            shell_print(shell, "[%-3d] %-35s: %-6s | %s", gpio_idx, gpio_name[gpio_idx], pin_dir, "resv");
        
        break;

    case GPIO_WRITE:
        if (!data)
            return 1;

        if ( *data!=0 && *data!=1 )
            return 1;

        if ( gpio_set(gpio_idx, *data) )
            return 1;
            
        break;
    
    default:
        LOG_ERR("No such mode %d!", mode);
        break;
    }
    
    return 0;
}

static int gpio_get_group_idx_by_dev_name(const char *dev_name)
{
    if (!dev_name)
        return -1;

    int group_idx = -1;
    for (int i=0; i<ARRAY_SIZE(GPIO_GROUP_NAME_LST); i++) {
        if (!strcmp(dev_name, GPIO_GROUP_NAME_LST[i]))
            group_idx = i;
    }

    return group_idx;
}

static const char* gpio_get_name(const char *dev_name, int pin_num)
{
    if (!dev_name)
        return -1;

    int name_idx = -1;
    name_idx = pin_num + 32 * gpio_get_group_idx_by_dev_name(dev_name);

    if (name_idx == -1)
        return NULL;

    if (name_idx >= GPIO_CFG_SIZE)
        return "Undefined";

    return gpio_name[name_idx];
}

/*********************************************************************************************************
 * COMMAND FUNCTION SECTION
**********************************************************************************************************/
/* 
    Command header 
*/
static int cmd_info_print(const struct shell *shell, size_t argc, char **argv)
{
    shell_print(shell, "========================{SHELL COMMAND INFO}========================");
    shell_print(shell, "* DESCRIPTION:   Platform command");
    shell_print(shell, "* AUTHOR:        MouchenHung");
    shell_print(shell, "* DATE/VERSION:  %s - %s", RELEASE_DATE, RELEASE_VERSION);
    shell_print(shell, "* Note:          You know, I'm Mouchen Hung, the most handsome one!!");
    shell_print(shell, "========================{SHELL COMMAND INFO}========================");
    return 0;
}

/* 
    Command GPIO 
*/
static void cmd_gpio_cfg_list_group(const struct shell *shell, size_t argc, char **argv)
{
    if (argc != 2) {
        shell_warn(shell, "Try: platform gpio list_group <gpio_device>");
        return;
    }

    const struct device *dev;
    dev = device_get_binding(argv[1]);

    if (!dev) {
        shell_error(shell, "Device [%s] not found!", argv[1]);
        return;
    }

    

    int g_idx = gpio_get_group_idx_by_dev_name(dev->name);
    int max_group_pin = num_of_pin_in_one_group_lst[g_idx];
        
    int rc;
    for (int index=0; index<max_group_pin; index++) {
#if PINMASK_RESERVE_CHECK
        /* avoid pin_mask from devicetree "gpio-reserved" */
        if (gpio_check_reserve(dev, index, CHECK_BY_GROUP_IDX)) {
            shell_print(shell, "[%-3d] %-35s: %s | <none>", index, "gpio_reserve", "i/o");
            continue;
        }
#endif
        char *pin_dir = "output";
        if ( gpio_cfg[g_idx * 32 + index].direction == GPIO_INPUT )
            pin_dir = "input";
            
        rc = gpio_pin_get(dev, index);
        if (rc >= 0) {
            shell_print(shell, "[%-3d] %-35s: %-6s | %d", index, gpio_get_name(dev->name, index), pin_dir, rc);
        } else {
            shell_error(shell, "[%-3d] %-35s: %-6s | err[%d]", index, gpio_get_name(dev->name, index), pin_dir, rc);
        }
    }

    return;
}

static void cmd_gpio_cfg_list_all(const struct shell *shell, size_t argc, char **argv)
{
    if (argc != 1) {
        shell_warn(shell, "Try: platform gpio list_all");
        return;
    }

    for (int gpio_idx=0; gpio_idx<GPIO_CFG_SIZE; gpio_idx++)
        gpio_access_cfg(shell, gpio_idx, GPIO_READ, NULL);

    return;
}

static void cmd_gpio_cfg_get(const struct shell *shell, size_t argc, char **argv)
{
    if (argc != 2) {
        shell_warn(shell, "Try: platform gpio get <gpio_idx>");
        return;
    }

    int gpio_index = strtol(argv[1], NULL, 10);
    if ( gpio_access_cfg(shell, gpio_index, GPIO_READ, NULL) )
        shell_error(shell, "gpio[%d] get failed!", gpio_index);

    return;
}

static void cmd_gpio_cfg_set_val(const struct shell *shell, size_t argc, char **argv)
{
    if (argc != 3) {
        shell_warn(shell, "Try: platform gpio set val <gpio_idx> <data>");
        return;
    }

    int gpio_index = strtol(argv[1], NULL, 10);
    int data = strtol(argv[2], NULL, 10);

    if ( gpio_access_cfg(shell, gpio_index, GPIO_WRITE, &data) )
        shell_error(shell, "gpio[%d] --> %d ,failed!", gpio_index, data);
    else
        shell_print(shell, "gpio[%d] --> %d ,success!", gpio_index, data);
    
    return;
}

static void cmd_gpio_cfg_set_dir(const struct shell *shell, size_t argc, char **argv)
{
    shell_warn(shell, "GPIO set DIR command not support!");

    /* TODO */

    return;
}

/* 
    Command I2C SLAVE
*/
//TODO

/*********************************************************************************************************
 * COMMAND DECLARE SECTION
**********************************************************************************************************/

/* GPIO sub command */
static void device_gpio_name_get(size_t idx, struct shell_static_entry *entry)
{
    const struct device *dev = shell_device_lookup(idx, GPIO_DEVICE_PREFIX);

    entry->syntax = (dev != NULL) ? dev->name : NULL;
    entry->handler = NULL;
    entry->help = NULL;
    entry->subcmd = NULL;
}
SHELL_DYNAMIC_CMD_CREATE(gpio_device_name, device_gpio_name_get);

SHELL_STATIC_SUBCMD_SET_CREATE(sub_gpio_set_cmds,
    SHELL_CMD(val, NULL, "Set pin value.", cmd_gpio_cfg_set_val),
    SHELL_CMD(dir, NULL, "Set pin direction.", cmd_gpio_cfg_set_dir),
    SHELL_SUBCMD_SET_END
);

SHELL_STATIC_SUBCMD_SET_CREATE(sub_gpio_cmds,
    SHELL_CMD(list_group, &gpio_device_name, "List all GPIO config from certain group.", cmd_gpio_cfg_list_group),
    SHELL_CMD(list_all, NULL, "List all GPIO config.", cmd_gpio_cfg_list_all),
    SHELL_CMD(get, NULL, "Get GPIO config", cmd_gpio_cfg_get),
    SHELL_CMD(set, &sub_gpio_set_cmds, "Set certain GPIO config", NULL),
    SHELL_SUBCMD_SET_END
);

/* I2C slave sub command */
// TODO

SHELL_STATIC_SUBCMD_SET_CREATE(sub_platform_cmds,
                    SHELL_CMD(note, NULL,
                        "Note list.", cmd_info_print),
                    
                    SHELL_CMD(gpio, &sub_gpio_cmds,
                        "GPIO relative command.", NULL),

                       SHELL_SUBCMD_SET_END
                   );

SHELL_CMD_REGISTER(platform, &sub_platform_cmds, "Platform commands", NULL);
