#include <iostream>
#include <windows.h>
#include <conio.h>
#include <atomic>

using namespace std;

atomic<bool> g_bRunning(true);
HWND g_hwnd = NULL; // 全局窗口句柄

// 消息处理函数
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_HOTKEY:
    if (wParam == 1)
    { // Ctrl+Alt+Q
      cout << "\n[退出] 检测到 Ctrl+Alt+Q，程序即将退出..." << endl;
      g_bRunning = false;
      PostQuitMessage(0);
      return 0;
    }
    else if (wParam == 2)
    { // Ctrl+Alt+M
      cout << "\n[提示] 你按下了 Ctrl+Alt+M 组合键！" << endl;
      cout << "[提示] 程序继续运行，按 Ctrl+Alt+Q 退出程序" << endl;
      return 0;
    }
    break;

  case WM_DESTROY:
    g_bRunning = false;
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// 热键监听线程
DWORD WINAPI HotkeyThread(LPVOID lpParam)
{
  // 创建消息窗口
  HINSTANCE hInstance = GetModuleHandle(NULL);

  // 注册窗口类
  const wchar_t *CLASS_NAME = L"SimpleHotkeyListener";

  WNDCLASSW wc = {};
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;

  if (!RegisterClassW(&wc))
  {
    DWORD error = GetLastError();
    if (error != 1410)
    { // 1410 表示类已经存在
      cerr << "窗口类注册失败！错误代码: " << error << endl;
      return 1;
    }
  }

  // 创建隐藏窗口
  g_hwnd = CreateWindowExW(
      0,
      CLASS_NAME,
      L"Hotkey Listener",
      0,
      0, 0, 0, 0,
      HWND_MESSAGE, // 消息专用窗口
      NULL,
      hInstance,
      NULL);

  if (g_hwnd == NULL)
  {
    cerr << "窗口创建失败！错误代码: " << GetLastError() << endl;
    return 1;
  }

  // 注册热键
  // Ctrl+Alt+Q 退出
  if (!RegisterHotKey(g_hwnd, 1, MOD_CONTROL | MOD_ALT, 'Q'))
  {
    cerr << "退出热键注册失败！错误代码: " << GetLastError() << endl;
    return 1;
  }

  // Ctrl+Alt+M 显示消息
  if (!RegisterHotKey(g_hwnd, 2, MOD_CONTROL | MOD_ALT, 'M'))
  {
    cerr << "消息热键注册失败！错误代码: " << GetLastError() << endl;
    // 继续运行，不影响退出热键
    cout << "提示: 消息热键注册失败，但退出热键仍然可用" << endl;
  }

  cout << "热键注册成功！" << endl;

  // 消息循环
  MSG msg = {};
  while (g_bRunning && GetMessage(&msg, NULL, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // 注销热键
  UnregisterHotKey(g_hwnd, 1);
  UnregisterHotKey(g_hwnd, 2);

  // 销毁窗口
  if (g_hwnd)
  {
    DestroyWindow(g_hwnd);
    g_hwnd = NULL;
  }

  return 0;
}

int main()
{
  // Set console title
  SetConsoleTitleW(L"Keyboard Hotkey Listener");

  // 隐藏光标
  CONSOLE_CURSOR_INFO cursorInfo;
  GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
  cursorInfo.bVisible = false;
  SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

  cout << "==========================================" << endl;
  cout << "  键盘组合键监听程序  " << endl;
  cout << "==========================================" << endl;
  cout << "Program started, listening for hotkeys..." << endl;
  cout << "Hotkey Settings:" << endl;
  cout << "  - Ctrl + Alt + Q: Exit program" << endl;
  cout << "  - Ctrl + Alt + M: Show message" << endl;
  cout << "  - ESC: Manual exit" << endl;
  cout << "------------------------------------------" << endl;
  cout << "Tip: Press a hotkey combination!" << endl;
  cout << "==========================================" << endl;

  // 创建热键监听线程
  HANDLE hThread = CreateThread(NULL, 0, HotkeyThread, NULL, 0, NULL);

  if (hThread == NULL)
  {
    cerr << "线程创建失败！错误代码: " << GetLastError() << endl;
    cout << "按任意键退出..." << endl;
    _getch();
    return 1;
  }

  // 主循环，检查ESC键
  while (g_bRunning)
  {
    if (_kbhit())
    {
      int ch = _getch();
      if (ch == 27)
      { // ESC键
        cout << "\n[退出] 检测到ESC键，程序退出..." << endl;
        g_bRunning = false;

        // 发送退出消息到窗口线程
        if (g_hwnd)
        {
          PostMessage(g_hwnd, WM_DESTROY, 0, 0);
        }
        break;
      }
    }
    Sleep(50); // 降低CPU占用
  }

  // 等待线程结束
  WaitForSingleObject(hThread, 2000);
  CloseHandle(hThread);

  // 恢复光标显示
  cursorInfo.bVisible = true;
  SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

  cout << "\n[完成] 程序已退出" << endl;
  cout << "按任意键关闭窗口..." << endl;
  _getch();

  return 0;
}