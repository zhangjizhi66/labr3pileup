
# LaBr3 波形解堆积算法

## 0Check

查看原始波形和快速滤波波形，生成 check_%04d_W1000.root 文件。

在生成文件的 h2fwhm 目录下检查波形半高宽与高度的关联关系, 并用 TCutG 选取中间的条带。

TCutG 命名规则："cut_%d_%d"，第一个整数为采集卡号det，第二个整数为通道数id

TCutG 文件名为 "cut_%d_%d.C"

## 1StandardWave

求标准波形, 并拟合波形半高宽与高度的关联关系, 将波形宽度归一.

## 2FitWave

拟合堆积波形, 给出解堆积后每个波形的位置和高度.

标准波形文件可选由该轮数据生成的 (USE_CURRENT_STDWAVE) 或某固定轮数据生成的 (USE_FIXED_STDWAVE)

## 参数

在 UserDefine 中调节程序参数