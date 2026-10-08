// NatLang Studio Easy — minimal, dependency-free Win32 frontend for natc.
// Copyright (c) 2026 Gabriele Viola and NatLang contributors. MIT License.
#define UNICODE
#define _UNICODE
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <memory>
#include <iterator>
#include <stdexcept>
#include <algorithm>

namespace fs=std::filesystem;
namespace {
constexpr UINT MSG_OUTPUT=WM_APP+40,MSG_DONE=WM_APP+41;
enum Id {SOURCE=101,OUTPUT,INPUT,NEWFILE,OPENFILE,SAVEFILE,CHECK,BUILD,RUN,PREVIEW,STARTAI,SENDINPUT,HELP,STOP};
HWND window=nullptr,editor=nullptr,logBox=nullptr,inputBox=nullptr,statusBox=nullptr;
HWND controls[10]{};
std::wstring filePath,appDir,compilerPath;
std::mutex inputMutex;
HANDLE runningStdin=nullptr;
HANDLE activeProcess=nullptr;
std::mutex activeMutex;
bool busy=false,dirty=false;

std::wstring utf16(const std::string &s) {
 if(s.empty()) return {};
 int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.data(),(int)s.size(),nullptr,0);
 if(n<=0) return L"[Output encoding error]";
 std::wstring w(n,L'\0');MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.data(),(int)s.size(),w.data(),n);return w;
}
std::string utf8(const std::wstring &s) {
 if(s.empty()) return {};
 int n=WideCharToMultiByte(CP_UTF8,0,s.data(),(int)s.size(),nullptr,0,nullptr,nullptr);
 std::string a(n,'\0');WideCharToMultiByte(CP_UTF8,0,s.data(),(int)s.size(),a.data(),n,nullptr,nullptr);return a;
}
std::wstring quote(std::wstring x){
 std::wstring out=L"\"";size_t slashes=0;
 for(auto c:x){if(c==L'\\'){++slashes;continue;}
  if(c==L'\"'){out.append(slashes*2+1,L'\\');out+=L'\"';slashes=0;continue;}
  out.append(slashes,L'\\');slashes=0;out+=c;
 }
 out.append(slashes*2,L'\\');return out+L'\"';
}
std::wstring textOf(HWND h){int n=GetWindowTextLengthW(h);std::wstring w(n+1,L'\0');GetWindowTextW(h,w.data(),n+1);w.resize(n);return w;}
void postOutput(const std::wstring& s){auto *p=new std::wstring(s);if(!PostMessageW(window,MSG_OUTPUT,0,reinterpret_cast<LPARAM>(p)))delete p;}
void append(HWND h,const std::wstring &s){int len=GetWindowTextLengthW(h);SendMessageW(h,EM_SETSEL,len,len);SendMessageW(h,EM_REPLACESEL,FALSE,(LPARAM)s.c_str());SendMessageW(h,EM_SCROLLCARET,0,0);}
void setBusy(bool b){busy=b;for(HWND c:controls)if(c&&GetDlgCtrlID(c)!=STOP)EnableWindow(c,!b);for(HWND c:controls)if(c&&GetDlgCtrlID(c)==STOP)EnableWindow(c,b);SetWindowTextW(statusBox,b?L"In esecuzione...":L"Pronto");}
std::wstring natc(){return (fs::path(appDir)/L"natc.exe").wstring();}
std::wstring destination(){fs::path p(filePath);p.replace_extension(L".exe");return p.wstring();}
std::wstring cwd(){return fs::path(filePath).parent_path().wstring();}

bool save(bool forceDialog=false){
 if(filePath.empty()||forceDialog){wchar_t dest[MAX_PATH]={};if(!filePath.empty())wcsncpy_s(dest,filePath.c_str(),_TRUNCATE);
  OPENFILENAMEW o{};o.lStructSize=sizeof(o);o.hwndOwner=window;o.lpstrFilter=L"NatLang (*.nat)\0*.nat\0Tutti i file (*.*)\0*.*\0\0";
  o.lpstrFile=dest;o.nMaxFile=MAX_PATH;o.lpstrDefExt=L"nat";o.Flags=OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT;
  if(!GetSaveFileNameW(&o))return false;filePath=dest;
 }
 try {std::ofstream out(fs::path(filePath),std::ios::binary);if(!out)throw std::runtime_error("save");out<<utf8(textOf(editor));if(!out)throw std::runtime_error("write");dirty=false;}
 catch(...){MessageBoxW(window,L"Non riesco a salvare il file.",L"NatLang",MB_ICONERROR);return false;}
 SetWindowTextW(window,(L"NatLang Studio Easy — "+filePath).c_str());return true;
}
void load(){
 wchar_t dest[MAX_PATH]={};OPENFILENAMEW o{};o.lStructSize=sizeof(o);o.hwndOwner=window;
 o.lpstrFilter=L"NatLang (*.nat)\0*.nat\0Tutti i file (*.*)\0*.*\0\0";o.lpstrFile=dest;o.nMaxFile=MAX_PATH;o.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
 if(!GetOpenFileNameW(&o))return;
 try {std::ifstream f(fs::path(dest),std::ios::binary);if(!f)throw std::runtime_error("open");std::string s((std::istreambuf_iterator<char>(f)),{});SetWindowTextW(editor,utf16(s).c_str());filePath=dest;dirty=false;SetWindowTextW(window,(L"NatLang Studio Easy — "+filePath).c_str());}
 catch(...){MessageBoxW(window,L"Impossibile leggere il file UTF-8.",L"NatLang",MB_ICONERROR);}
}
struct ProcResult {DWORD code=1;bool started=false;};
ProcResult execute(const std::wstring &exe,const std::vector<std::wstring> &args,bool interactive){
 SECURITY_ATTRIBUTES sa{sizeof(sa),nullptr,TRUE};HANDLE readOut=nullptr,writeOut=nullptr,readIn=nullptr,writeIn=nullptr;
 if(!CreatePipe(&readOut,&writeOut,&sa,0)){postOutput(L"Impossibile creare pipe di output.\r\n");return {};}
 SetHandleInformation(readOut,HANDLE_FLAG_INHERIT,0);
 if(!CreatePipe(&readIn,&writeIn,&sa,0)){CloseHandle(readOut);CloseHandle(writeOut);return {};}
 SetHandleInformation(writeIn,HANDLE_FLAG_INHERIT,0);
 STARTUPINFOW si{};si.cb=sizeof(si);si.dwFlags=STARTF_USESTDHANDLES;si.hStdOutput=writeOut;si.hStdError=writeOut;si.hStdInput=readIn;
 PROCESS_INFORMATION pi{};
 std::wstring cmd=quote(exe);for(auto &a:args)cmd+=L" "+quote(a);
 std::wstring working=cwd();
 BOOL ok=CreateProcessW(exe.c_str(),cmd.data(),nullptr,nullptr,TRUE,CREATE_NO_WINDOW,nullptr,working.empty()?nullptr:working.c_str(),&si,&pi);
 CloseHandle(writeOut);CloseHandle(readIn);
 if(!ok){postOutput(L"Impossibile avviare processo (errore Windows "+std::to_wstring(GetLastError())+L").\r\n");CloseHandle(readOut);CloseHandle(writeIn);return {};}
 {std::lock_guard<std::mutex> lock(activeMutex);activeProcess=pi.hProcess;}
 if(interactive){std::lock_guard<std::mutex> lock(inputMutex);runningStdin=writeIn;}
 else CloseHandle(writeIn);
 char buf[4096];DWORD n=0;
 while(ReadFile(readOut,buf,sizeof(buf),&n,nullptr)&&n){
   std::string data(buf,n);std::wstring w=utf16(data);
   if(w==L"[Output encoding error]"){w=L"[Output non UTF-8: impossibile visualizzare alcuni caratteri]";}
   postOutput(w);
 }
 WaitForSingleObject(pi.hProcess,INFINITE);
 DWORD code=1;GetExitCodeProcess(pi.hProcess,&code);
 if(interactive){std::lock_guard<std::mutex> lock(inputMutex);runningStdin=nullptr;CloseHandle(writeIn);}
 CloseHandle(readOut);CloseHandle(pi.hThread);
 {std::lock_guard<std::mutex> lock(activeMutex);activeProcess=nullptr;CloseHandle(pi.hProcess);}
 return {code,true};
}
void startWork(int id){
 if(busy)return;
 if(!save())return;
 if(!fs::exists(natc())){MessageBoxW(window,L"natc.exe non trovato accanto a NatLang Studio. Usa il pacchetto Easy completo.",L"NatLang",MB_ICONERROR);return;}
 SetWindowTextW(logBox,L"");setBusy(true);
 const std::wstring src=filePath,output=destination(),comp=compilerPath;
 std::thread([id,src,output,comp](){
  std::vector<std::wstring> args{src};
  bool requiresToolchain=(id==BUILD||id==RUN);
  if(id==CHECK)args.push_back(L"--check");
  if(id==PREVIEW)args.push_back(L"--llm-preview");
  if(requiresToolchain){args.insert(args.end(),{L"-o",output,L"--opt-level",L"0"});
   if(!comp.empty())args.insert(args.end(),{L"--compiler",comp});
  }
  postOutput(L"> natc "+src+L"\r\n");
  ProcResult res=execute(natc(),args,false);
  if(res.started&&res.code==0&&id==RUN){postOutput(L"\r\n--- Esecuzione programma (invia input in basso se richiesto) ---\r\n");res=execute(output,{},true);}
  if(!res.started||res.code!=0)postOutput(L"\r\nOperazione non riuscita (exit code "+std::to_wstring(res.code)+L").\r\n");
  else postOutput(L"\r\nOperazione completata.\r\n");
  PostMessageW(window,MSG_DONE,res.code,0);
 }).detach();
}
void stopWork(){
 std::lock_guard<std::mutex> lock(activeMutex);
 if(activeProcess)TerminateProcess(activeProcess,1223);
}
void sendInput(){
 std::wstring w=textOf(inputBox)+L"\n";std::string raw=utf8(w);
 bool ok=false;{std::lock_guard<std::mutex> lock(inputMutex);if(runningStdin){DWORD n=0;ok=WriteFile(runningStdin,raw.data(),(DWORD)raw.size(),&n,nullptr)!=0;}}
 if(ok){SetWindowTextW(inputBox,L"");append(logBox,L"> "+w);}else MessageBoxW(window,L"Avvia prima un programma che richiede input.",L"NatLang",MB_OK|MB_ICONINFORMATION);
}
void startAI(){
 fs::path script=fs::path(appDir)/L"scripts"/L"start_gguf.ps1";
 if(!fs::exists(script)){MessageBoxW(window,L"Script AI non incluso nel pacchetto.",L"NatLang",MB_ICONERROR);return;}
 std::wstring args=L"-NoExit -NoProfile -ExecutionPolicy Bypass -File "+quote(script.wstring())+L" -Model qwen3";
 auto h=ShellExecuteW(window,L"open",L"powershell.exe",args.c_str(),appDir.c_str(),SW_SHOWNORMAL);
 if((INT_PTR)h<=32)MessageBoxW(window,L"Non riesco ad avviare PowerShell.",L"NatLang",MB_ICONERROR);
 else append(logBox,L"Server AI: si apre una finestra PowerShell. Il primo avvio scarica il modello GGUF.\r\n");
}
HWND button(int id,const wchar_t *caption,int x,int y,int width){HWND h=CreateWindowW(L"BUTTON",caption,WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,x,y,width,30,window,(HMENU)(INT_PTR)id,nullptr,nullptr);SendMessageW(h,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),TRUE);return h;}
void layout(int width,int height){
 int pad=12,bar=43;int edH=std::max(150,(height-170)*2/3);
 MoveWindow(editor,pad,bar,width-pad*2,edH,TRUE);
 int logY=bar+edH+10, inputY=height-45;
 MoveWindow(logBox,pad,logY,width-pad*2,std::max(50,inputY-logY-10),TRUE);
 MoveWindow(inputBox,pad,inputY,width-148,27,TRUE);
 MoveWindow(GetDlgItem(window,SENDINPUT),width-128,inputY,116,27,TRUE);
}
LRESULT CALLBACK procedure(HWND h,UINT msg,WPARAM wp,LPARAM lp){
 switch(msg){
 case WM_CREATE:{
  window=h;int x=12;
  struct Item {int id;const wchar_t *txt;int w;};
  const Item b[]={{NEWFILE,L"Nuovo",60},{OPENFILE,L"Apri",60},{SAVEFILE,L"Salva",60},{CHECK,L"Verifica",82},{BUILD,L"Compila",82},{RUN,L"Esegui",82},{STOP,L"Stop",60},{PREVIEW,L"Anteprima AI",110},{STARTAI,L"Avvia AI",90},{HELP,L"Guida",60}};
  for(size_t i=0;i<std::size(b);++i){HWND z=button(b[i].id,b[i].txt,x,7,b[i].w);controls[i]=z;x+=b[i].w+7;}
  editor=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"Mostra 2 più 2\r\n",WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_MULTILINE|ES_AUTOVSCROLL|ES_WANTRETURN,12,43,1000,350,h,(HMENU)SOURCE,nullptr,nullptr);
  logBox=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"Output e messaggi del compilatore...",WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_MULTILINE|ES_AUTOVSCROLL|ES_READONLY,12,420,1000,150,h,(HMENU)OUTPUT,nullptr,nullptr);
  inputBox=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"",WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,12,700,700,27,h,(HMENU)INPUT,nullptr,nullptr);
  button(SENDINPUT,L"Invia input",900,700,115);
  HFONT font=(HFONT)GetStockObject(ANSI_FIXED_FONT);for(HWND z:{editor,logBox,inputBox})SendMessageW(z,WM_SETFONT,(WPARAM)font,TRUE);
  SendMessageW(editor,EM_SETLIMITTEXT,1000000,0);
  statusBox=CreateWindowW(L"STATIC",L"Pronto",WS_CHILD|WS_VISIBLE,905,12,200,24,h,nullptr,nullptr,nullptr);
  fs::path exe;wchar_t filename[32768];GetModuleFileNameW(nullptr,filename,(DWORD)std::size(filename));exe=filename;appDir=exe.parent_path().wstring();
  auto cc=fs::path(appDir)/L"toolchain"/L"bin"/L"clang++.exe";
  if(fs::exists(cc)){compilerPath=cc.wstring();std::wstring path(32768,L'\0');DWORD n=GetEnvironmentVariableW(L"PATH",path.data(),(DWORD)path.size());path.resize(n<path.size()?n:0);SetEnvironmentVariableW(L"PATH",(cc.parent_path().wstring()+L";"+path).c_str());}
  setBusy(false);return 0;}
 case WM_SIZE:layout(LOWORD(lp),HIWORD(lp));return 0;
 case WM_COMMAND:{int id=LOWORD(wp);if(id==SOURCE&&HIWORD(wp)==EN_CHANGE){dirty=true;return 0;}
  if(id==SAVEFILE)save();else if(id==OPENFILE&&!busy)load();else if(id==NEWFILE&&!busy){filePath.clear();SetWindowTextW(editor,L"Mostra 2 più 2\r\n");SetWindowTextW(window,L"NatLang Studio Easy — nuovo programma");}
  else if(id==CHECK||id==BUILD||id==RUN||id==PREVIEW)startWork(id);
  else if(id==SENDINPUT)sendInput();else if(id==STOP)stopWork();else if(id==STARTAI)startAI();
  else if(id==HELP)ShellExecuteW(h,L"open",(fs::path(appDir)/L"docs"/L"EASY_START.md").wstring().c_str(),nullptr,appDir.c_str(),SW_SHOWNORMAL);
  return 0;}
 case MSG_OUTPUT:{std::unique_ptr<std::wstring> t((std::wstring*)lp);append(logBox,*t);return 0;}
 case MSG_DONE:setBusy(false);SetWindowTextW(statusBox,wp==0?L"Completato":L"Errore: vedi output");return 0;
 case WM_CLOSE:if(busy){MessageBoxW(h,L"Attendi il completamento dell'operazione o termina NatLang Studio dal sistema.",L"NatLang",MB_OK|MB_ICONINFORMATION);return 0;}DestroyWindow(h);return 0;
 case WM_DESTROY:PostQuitMessage(0);return 0;
 }
 return DefWindowProcW(h,msg,wp,lp);
}
}
int WINAPI wWinMain(HINSTANCE inst,HINSTANCE,LPWSTR,int show){
 WNDCLASSEXW cls{};cls.cbSize=sizeof(cls);cls.lpfnWndProc=procedure;cls.hInstance=inst;cls.lpszClassName=L"NatLangStudioWindow";cls.hCursor=LoadCursorW(nullptr,IDC_ARROW);cls.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);cls.hIcon=LoadIconW(nullptr,IDI_APPLICATION);
 if(!RegisterClassExW(&cls))return 1;
 HWND h=CreateWindowExW(0,cls.lpszClassName,L"NatLang Studio Easy",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,1120,800,nullptr,nullptr,inst,nullptr);
 if(!h)return 1;ShowWindow(h,show);UpdateWindow(h);
 MSG m;while(GetMessageW(&m,nullptr,0,0)>0){TranslateMessage(&m);DispatchMessageW(&m);}return 0;
}
