#ifndef SHELL_PLATFORM_H
#define SHELL_PLATFORM_H

/* Declare Common */
#define sensor_name_to_num(x) #x,
#define GET_BIT_VAL(val, n) ( (val&BIT(n))>>(n) )

/* Declare GPIO */
#define PINMASK_RESERVE_CHECK 1
#define GPIO_DEVICE_PREFIX "GPIO0_"
#define GPIO_RESERVE_PREFIX "Reserve"
#define NUM_OF_GROUP 6
#define NUM_OF_GPIO_IS_DEFINE 167
#define REG_GPIO_BASE 0x7e780000
#define REG_SCU 0x7E6E2000

int num_of_pin_in_one_group_lst[NUM_OF_GROUP] = {32, 32, 32, 32, 32, 16};
char GPIO_GROUP_NAME_LST[NUM_OF_GROUP][10] = {"GPIO0_A_D", "GPIO0_E_H", "GPIO0_I_L", "GPIO0_M_P", "GPIO0_Q_T", "GPIO0_U_V"};

uint32_t GPIO_GROUP_REG_ACCESS[NUM_OF_GROUP] = {
    REG_GPIO_BASE+0x00,
    REG_GPIO_BASE+0x20,
    REG_GPIO_BASE+0x70,
    REG_GPIO_BASE+0x78,
    REG_GPIO_BASE+0x80,
    REG_GPIO_BASE+0x88
};

uint32_t GPIO_MULTI_FUNC_PIN_CTL_REG_ACCESS[] = {
    REG_SCU+0x410,
    REG_SCU+0x414,
    REG_SCU+0x418,
    REG_SCU+0x41C,
    REG_SCU+0x430,
    REG_SCU+0x434,
    REG_SCU+0x510,
    REG_SCU+0x51C,
};

enum GPIO_ACCESS {
    GPIO_READ,
    GPIO_WRITE
};

/* Declare GPIO */
const char * const sensor_type_name[] = {
    sensor_name_to_num(tmp75)
    sensor_name_to_num(adc)
    sensor_name_to_num(peci)
    sensor_name_to_num(vr)
    sensor_name_to_num(hsc)
    sensor_name_to_num(nvme)
    sensor_name_to_num(pch)
};

const char * const sensor_status_name[] = {
    sensor_name_to_num(read_success)
    sensor_name_to_num(read_acur_success)
    sensor_name_to_num(not_found)
    sensor_name_to_num(not_accesible)
    sensor_name_to_num(fail_to_access)
    sensor_name_to_num(init_status)
    sensor_name_to_num(unspecified_err)
    sensor_name_to_num(polling_disable)
};

enum SENSOR_ACCESS {
    SENSOR_READ,
    SENSOR_WRITE
};

#endif