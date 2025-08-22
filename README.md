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

## 如何使用
1. 通过Code下载压缩包解压使用
2. 使用git clone部署至本地

## 鸣谢

- github用户[[bdth-7777777]](https://github.com/bdth-7777777): https://github.com/bdth-7777777/OLED_UI
- 图标库: https://igoutu.cn/icons
- 图标库: https://github.com/hugeicons/hugeicons-react
- 图片取模工具：https://javl.github.io/image2cpp/
