" Поиск относительно текущего каталога
if !exists("g:cpp_inc_dirs")
	let g:cpp_inc_dirs = '-I../include -I../lib'
endif

" Переключение по F4 на заголовки
let b:fswitchdst = 'cpp'
let b:fswitchlocs = '../src'

" Загрузить настройки
source ../.vim

