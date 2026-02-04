# Cursor 里“改完就丢”的说明与设置

## 为什么会消失？

在 Cursor 的 **Composer / Chat** 里，AI 对文件的修改流程是：

1. **Apply（应用）** — 把建议写进文件，此时编辑器里会出现 **diff（对比）**
2. **Accept / Reject（保留 / 拒绝）** — 你必须在这步选“保留”，修改才会真正落盘

如果只点了 Apply、没有点 **Accept（或按快捷键）**，关掉 diff 或切走之后，未接受的修改会被丢弃，所以会出现“改完又没了”的情况。

---

## 有没有“默认保留”、不用按 Keep？

**没有。**  
Cursor 目前**没有**“自动接受所有修改”的选项，所有通过 Composer/Chat 应用的修改，最后都要你**手动 Accept** 一次才会永久保存。

所以：**不能** 设成“默认就等于按了 Keep”，只能尽量少按一次、或别忘按。

---

## 可以做的几件事（少丢改、少忘按）

### 1. 用快捷键接受（推荐）

- **Windows / Linux**：`Ctrl + Enter` 接受当前修改  
- **macOS**：`Cmd + Enter` 接受当前修改  

Apply 之后立刻按一次，养成习惯，就不容易忘。

### 2. 打开“自动应用到上下文外文件”（可选）

- 打开 **Cursor 设置**（`Ctrl+,` / `Cmd+,`）
- 搜索：`Auto-Apply` 或 `apply` 或 `Composer`
- 找到类似 **“Auto-Apply to Files Outside Context”** 的选项并开启  

这样 AI 会把修改直接应用到不在当前上下文里的文件，你**仍然要在出现的 diff 里按 Accept（或 Ctrl/Cmd+Enter）**，否则还是不会保存。这个选项只是减少你手动“应用”的步骤，**不会**代替“接受”。

### 3. 重要改动：Apply 后马上 Accept

- 每次看到 Composer 里出现“已修改文件”的 diff，**立刻**在 diff 上点 **Accept** 或按 **Ctrl+Enter / Cmd+Enter**
- 或者先 **Save All**（`Ctrl+K S` / `Cmd+K S`），有时会促使未决修改被确认（具体行为以当前 Cursor 版本为准）

### 4. 用 Git 兜底

- 重要修改前先 `git add` / `git commit` 一次  
- 若发现“改完又没了”，可用 `git status`、`git diff` 或 `git checkout -- <file>` 查看/恢复

---

## 小结

| 问题 | 结论 |
|------|------|
| 能不能“默认 Keep”、不用按？ | **不能**，没有该设置 |
| 最少要做什么才不会丢？ | Apply 之后**必须再按一次 Accept**（或 Ctrl/Cmd+Enter） |
| 怎么少忘？ | 养成 Apply 后立刻 **Ctrl+Enter / Cmd+Enter** 的习惯 |

本说明放在 `项目剖析与追踪/` 下，方便以后查阅；若 Cursor 以后增加“默认接受”选项，再更新此文档即可。
