# Listen — 英语听力单选题固件

面向 EasyInput V2.0 / AI Keyboard V2.1（ESP32-S3R8，8 MB PSRAM / 16 MB Flash）的离线英语听力练习固件。

## 操作

| 输入 | 功能 |
|---|---|
| 旋钮旋转 | 1～20 循环选题，切题后播报“第几题” |
| S1 | 播放英文题干 |
| S2 | 顺序播放 4 个英文选项 |
| S3 | 播放题干中文翻译 |
| S4 | 顺序播放 4 个选项中文翻译 |
| S5～S8 | 分别播放选项 1～4 |
| S9 / 旋钮按压短按 | 播放提示后进入语音答题窗口 |

语音回答只识别中文“一、二、三、四”，分别对应选项 1～4。识别失败不判题、不加分，可再按 S9 重试。

## 计分

| 是否听过中文翻译 | 答对 | 答错 |
|---|---:|---:|
| 没听中文翻译 | +5 | +0 |
| 只听题干翻译 | +4 | +1 |
| 只听选项翻译 | +3 | +1 |
| 两部分都听 | +2 | +1 |

同一题可以重复回答并重复得分；总分达到 100 后继续答题但不再增加总分。播放英文题干和英文选项不影响分数。切换题目时会清空离开题与进入题的翻译收听状态。

## 构建

项目文件全部位于本 `listen/` 目录，仓库根目录不放业务文件。

```bash
cd listen
idf.py set-target esp32s3
idf.py build
```

`idf.py flash` 会同时烧录应用、ESP-SR 的 `model` 分区，以及 `assets` SPIFFS 音频资源分区。

如需重新生成 335 个 WAV 音频，需在 macOS 上执行：

```bash
python3 tools/generate_assets.py
python3 tools/generate_assets.py --check-only
```

生成结果为 8 kHz、mono、unsigned 8-bit PCM，总计约 5.7 MB。

## 依赖说明

`espressif/esp-sr` 通过 Component Registry 声明在 `main/idf_component.yml`。在新的干净环境中首次配置会下载官方组件和模型包；`esp-sr` 2.6.0 的 Registry 压缩包约 184 MB，网络较慢时需要耐心等待。

## 已知验证边界

- 语音识别使用 ESP-SR 离线中文 MultiNet，命令词为 `yi/er/san/si`。
- 本实现采用按钮触发、麦克风 I2S 直采后送 MultiNet；未使用 AFE 降噪/回声消除。由于提示音播放结束后才开始录音，一般可避开扬声器串扰，但一、二、三、四都是单音节，实际识别率需要真板实测。
- 若实测识别率不理想，优先调整说话距离/环境噪声和 `main/voice_answer.c` 中的检测阈值；仍不理想时再把该模块升级为 AFE + MultiNet。
