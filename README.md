# Perfei_Remote
## 简介

>此项目根据B站开源大佬Fei_行家所发布的Remote_V3.0代码的基础移植了[[bdth-7777777]](https://github.com/bdth-7777777)大佬的[OLED_UI](https://github.com/bdth-7777777/OLED_UI)并优化了nrf发收包与无人机电压获取。

## 注意事项
- 请至少焊接使用一个按键(最好为```key1```),否则将无法与无人机通信
- 如果未使用key1请自行至文件夹```oled_ui```下的```OLED_UI_Driver.h```内将下列修改为你所焊接的按钮对应的引脚
```
#define Key_GetEnterStatus()    GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13)
```
- 或者将```main.c```里的```NrfTxPacket();```取消注释即可与无人机通信(此操作会导致主菜单帧数从60fps降至40fps)

- 按键
key1(确认)		key4(上一层)
key2(返回)		key3(下一层)

## 如何使用
1. 通过Code下载压缩包解压使用
2. 使用git clone部署至本地

## 已支持功能
- [x] 基础对频、飞控摇杆电压、摇杆ADC值
- [ ] 对飞控写入PID参数
- [ ] 获取飞机MPU6050姿态
- [ ] 定高定点
      
## 鸣谢

- github用户[[bdth-7777777]](https://github.com/bdth-7777777): https://github.com/bdth-7777777/OLED_UI
- 图标库: https://igoutu.cn/icons
- 图标库: https://github.com/hugeicons/hugeicons-react
- 图片取模工具：https://javl.github.io/image2cpp/
