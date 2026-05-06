# 格温小姐大冒险

这是一个 C++ EasyX 课程大作业项目。

## 项目版本

v1.0：项目初始版本。

v2.0：正式素材版，使用 `assets` 文件夹中的正式图片素材，删除临时地图、临时人物和临时技能绘制。

v3.0：新增 J 键防御迷雾技能。玩家按 J 后会在角色周围生成蓝色防御迷雾，持续约 1 秒，期间不会受到敌方技能伤害。技能有约 5 秒冷却时间，UI 会显示防御状态。

v4.0：新增游戏音效系统。游戏开始、玩家受伤、J 防御释放、游戏失败时会播放对应 wav 音效，增强游戏反馈。

v5.0：新增右键点击移动。

v6.0：新增光辉 E 预警音效。光辉 E 预警圈出现时会播放 `lux.wav`，提醒玩家及时躲避。

## 开发环境

- Visual Studio 2026
- C++17
- EasyX
- Windows

## 游戏操作

- Enter：开始游戏
- WASD / 方向键：移动角色
- J：释放防御迷雾，短时间免疫伤害
- R：胜利或失败后重新开始
- Esc：退出游戏

## 资源说明

游戏图片素材放在 `assets` 文件夹中。

- `assets/cover.png`：开始界面
- `assets/coverend.png`：结束界面
- `assets/map2.png`：游戏地图
- `assets/格温状态图.png`：玩家正常状态
- `assets/格温受击图.png`：玩家受伤状态
- `assets/ezq.png`：EZ Q 技能
- `assets/aceyr.png`：寒冰大招
- `assets/Lux1.png`：光辉 E 预警
- `assets/Lux2.png`：光辉 E 爆炸
- `assets/hp.png`：血量图标
- `assets/w_mist.png`：J 防御迷雾特效图片

## 音效说明

- `assets/sounds/我觉得ok.wav`：开始游戏音效
- `assets/sounds/啊！！惨叫.wav`：玩家受击音效
- `assets/sounds/gwen_W.wav`：J 防御释放音效
- `assets/sounds/lux.wav`：光辉 E 预警音效
- `assets/sounds/超级玛丽死亡音效.wav`：游戏失败音效

如果运行时提示“图片资源加载失败，请检查 assets 文件夹”，请检查图片文件是否完整，文件名是否和代码中的路径一致。
