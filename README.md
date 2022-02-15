# PROJ_Q_YV3p5_Shell_Platform_Command
Platform command with zephyr
### USAGE
- GPIO relative
  - Get all gpios' info\
    **platform gpio list_all**\
    ![alt text](./img/GPIO_listall.png "gpio list all")
    
  - Get all gpios' info in one group\
    **platform gpio list_group <gpio_group_name>**\
    ![alt text](./img/GPIO_listgroup.png "gpio list group")
    
  - Get one gpio info\
    **platform gpio get <gpio_num>**
    
  - Set one gpio's value\
    **platform gpio set val <gpio_num> <0/1>**\
    ![alt text](./img/GPIO_getset.png "gpio get/set")
    
  - Set one gpio's direction(not support!)\
    **platform gpio set dir <gpio_num> <0/1>**

- SENSOR relative
  - Get all sensors' info\
    **platform sensor list_all**
  
  - Get one sensor info\
    **platform sensor get <sensor_num>**

- I2C relative\
  Not support!

### NOTE
