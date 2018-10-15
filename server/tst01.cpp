/*
 * SIGTERM  Запрос завершения программы, отправляется программе
 * SIGSEGV  Некорректное обращение к памяти (ошибка сегментации)
 * SIGINT   Внешнее прерывание, обычно вызывается пользователем
 * SIGILL   Неправильный образ программы, например некорректная инструкция
 * SIGABRT  Аварийное завершение программы, например вызванное вызовом std::abort()
 * SIGFPE   Ошибочная арифметическая операция, например деление на ноль
 */
#include <csignal>
#include <iostream>
#include <thread>
#include <chrono>
using namespace std::chrono_literals;

namespace
{
  volatile std::sig_atomic_t gSignalStatus;
}

void sig_handler(int signal)
{
  gSignalStatus = signal;
}

int main( int, char** )
{
  // Установка обработчика сигнала
  if (SIG_ERR == std::signal(SIGTERM, sig_handler)
    std::


  std::cout << "Значение сигнала: " << gSignalStatus << '\n';
  std::cout << "Передаём сигнал " << SIGINT << '\n';
  std::raise(SIGINT);
  std::cout << "Значение сигнала: " << gSignalStatus << '\n';
  while(1) std::this_thread::sleep_for(50ms);
  std::exit(EXIT_SUCCESS);
}
