vim.cmd [[
" let $FZF_DEFAULT_COMMAND = 'rg --files --hidden --no-ignore --follow --glob "!.git/*" --glob "!build/*"'
"set tags=tags,./tags
" set makeprg=ninja\ -j3\ -C\ build/
"set makeprg=cmake\ --build\ build/
"set fixendofline
"let g:cmake_build_dir="./"

set path=.,../build/vcpkg_installed/x64-linux/include,/usr/include

]]

vim.opt.fixendofline = true
vim.opt.tags = { "tags", "./tags" }
-- vim.opt.makeprg = "cmake\\ --build\\ build/"
vim.opt.makeprg = "ninja -j3 -C ../build/"
