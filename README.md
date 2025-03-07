# TRICKRIG
## минималистичный 3D движок

<a title="CMake status" href="https://github.com/bigov/trickrig/actions?query=workflow%3ACMake"><img alt="CMake workflow Status" src="https://github.com/bigov/trickrig/workflows/CMake/badge.svg"></a>

Trickrig - это разрабатываемое на C++ ядро минималистичного графического 3D движка на основе _OpenGL_, с ипользованием свободных библиотек _glfw3, libpng16, sqlite3, glm_. В движке реализована многопоточность, минималистичное меню не привязанное к внешним библиотекам, заложена возможность реализации LOD.
 
Разрабатываемый код является частично мультиплатфоменным - поддерживает Linux и MS-Windows. Сборка на платформе _MS-Windows_ производится с использованием свободных инструментов и библиотек из состава MSYS2. На платформе _Linux_ сборки выполняется с использованием "стандартных" средств разработки.

При запуске приложения на ноутбуках необходимо назначать для приложения использование дискретной графической карты с поддержкой OpenGL 3.3.

![demo](demo0.png)

TrickRig при перемещении камеры обеспечивает динамическое перестроение данных OpenGL VAO при рендере активной сцены, что обеспечивает эффективное использование графической памяти приложения, и позволяет генерировать "бесконечные" открытые 3D пространства.

Подробности на сайте [bigov.github.io](https://bigov.github.io)
