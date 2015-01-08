set expandtab shiftwidth=2 softtabstop=2
set cindent cinoptions=>4,n-2,^-2,\:0,=2,g0,h2,t0,+2,(0,u0,w1,m1

map <F5> :Make<CR>

au! BufEnter *.cpp,*.cxx,*.cc,*.c let b:fswitchdst = 'h,hpp' | let b:fswitchlocs = 'reg:/src/include/,reg:|src|include/**|,ifrel:|/src/|../include|'
au! BufEnter *.h,*.hpp let b:fswitchdst = 'cc,cpp,c' | let b:fswitchlocs = 'reg:/include/src/,reg:/include.*/src/,ifrel:|/include/|../src|'

let g:ycm_global_ycm_extra_conf = '.ycm_extra_conf.py'
