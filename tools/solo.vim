" Vim syntax file
" Language:    Solo
" Maintainer:  Toralf Wittner <toralf.wittner@gmail.com>
"              modified by Frederic Gingras <frederic@gingras.cc>

if version < 600
    syntax clear
elseif exists("b:current_syntax")
    finish
endif

" Highlight superfluous closing parens, brackets and braces.
syn match soloError "]\|}\|)"

let s:builtins_map = {
    \ "Constant":  "nil",
    \ "Boolean":   "true false",
    \ "Cond":      "if if-not when when-not cond",
    \ "Exception": "try catch finally throw",
    \ "Repeat":    "map reduce filter for",
    \ "Special":   "def! def do fn* fn if let quote defmacro!",
    \ "Variable":  "*FOLDER* *ARGS* *PATH_SEPARATOR*",
    \ "Define":    "defn defmacro",
    \ "Macro":     "and or -> ->>",
    \ "Func":      "= not= not nil? false? true? symbol symbol? "
    \            . "string? symbol? map? seq? vector? keyword? var? "
    \            . "special-symbol? apply partial comp constantly "
    \            . "identity comparator fn? re-matcher re-find re-matches "
    \            . "re-groups re-seq re-pattern str pr prn print "
    \            . "println pr-str prn-str print-str println-str newline "
    \            . "macroexpand macroexpand-1 monitor-enter monitor-exit "
    \            . "eval find-doc file-seq flush hash load load-file "
    \            . "read read-line scan slurp subs sync test "
    \            . "format printf loaded-libs use require load-reader "
    \            . "load-string + - * / +' -' *' /' < <= == >= > dec dec' "
    \            . "inc inc' min max"
    \ }

for category in keys(s:builtins_map)
  let words = split(s:builtins_map[category], " ")
  "let words = map(copy(words), '"solo.core/" . v:val') + words
  let s:builtins_map[category] = words
endfor

for [category, words] in items(s:builtins_map)
  if words != []
    execute "syntax keyword solo" . category . " " . join(words, " ")
  endif
endfor

syn cluster soloAtomCluster   contains=soloError,soloFunc,soloMacro,soloCond,soloDefine,soloRepeat,soloException,soloConstant,soloVariable,soloSpecial,soloKeyword,soloString,soloCharacter,soloNumber,soloBoolean,soloQuote,soloUnquote,soloDispatch,soloPattern
syn cluster soloTopCluster    contains=@soloAtomCluster,soloComment,soloSexp,soloAnonFn,soloVector,soloMap,soloSet

syn keyword soloTodo contained FIXME XXX TODO FIXME: XXX: TODO:
syn match   soloComment contains=soloTodo ";.*$"

syn match   soloKeyword "\c:\{1,2}[a-z0-9?!\-_+*.=<>#$]\+\(/[a-z0-9?!\-_+*.=<>#$]\+\)\?"

syn region  soloString start=/L\="/ skip=/\\\\\|\\"/ end=/"/

syn match   soloCharacter "\\."
syn match   soloCharacter "\\[0-7]\{3\}"
syn match   soloCharacter "\\u[0-9]\{4\}"
syn match   soloCharacter "\\space"
syn match   soloCharacter "\\tab"
syn match   soloCharacter "\\newline"
syn match   soloCharacter "\\return"
syn match   soloCharacter "\\backspace"
syn match   soloCharacter "\\formfeed"

let radixChars = "0123456789abcdefghijklmnopqrstuvwxyz"
for radix in range(2, 36)
	execute 'syn match soloNumber "\c\<-\?' . radix . 'r['
				\ . strpart(radixChars, 0, radix)
				\ . ']\+\>"'
endfor

syn match   soloNumber "\<-\=[0-9]\+\(\.[0-9]*\)\=\(M\|\([eE][-+]\?[0-9]\+\)\)\?\>"
syn match   soloNumber "\<-\=[0-9]\+N\?\>"
syn match   soloNumber "\<-\=0x[0-9a-fA-F]\+\>"
syn match   soloNumber "\<-\=[0-9]\+/[0-9]\+\>"

syn match   soloQuote "\('\|`\)"
syn match   soloUnquote "\(\~@\|\~\)"
syn match   soloDispatch "\(#^\|#'\)"
syn match   soloDispatch "\^"

syn match   soloAnonArg contained "%\(\d\|&\)\?"
syn match   soloVarArg contained "&"

syn region soloSexpLevel0 matchgroup=soloParen0 start="(" matchgroup=soloParen0 end=")"           contains=@soloTopCluster,soloSexpLevel1
syn region soloSexpLevel1 matchgroup=soloParen1 start="(" matchgroup=soloParen1 end=")" contained contains=@soloTopCluster,soloSexpLevel2
syn region soloSexpLevel2 matchgroup=soloParen2 start="(" matchgroup=soloParen2 end=")" contained contains=@soloTopCluster,soloSexpLevel3
syn region soloSexpLevel3 matchgroup=soloParen3 start="(" matchgroup=soloParen3 end=")" contained contains=@soloTopCluster,soloSexpLevel4
syn region soloSexpLevel4 matchgroup=soloParen4 start="(" matchgroup=soloParen4 end=")" contained contains=@soloTopCluster,soloSexpLevel5
syn region soloSexpLevel5 matchgroup=soloParen5 start="(" matchgroup=soloParen5 end=")" contained contains=@soloTopCluster,soloSexpLevel6
syn region soloSexpLevel6 matchgroup=soloParen6 start="(" matchgroup=soloParen6 end=")" contained contains=@soloTopCluster,soloSexpLevel7
syn region soloSexpLevel7 matchgroup=soloParen7 start="(" matchgroup=soloParen7 end=")" contained contains=@soloTopCluster,soloSexpLevel8
syn region soloSexpLevel8 matchgroup=soloParen8 start="(" matchgroup=soloParen8 end=")" contained contains=@soloTopCluster,soloSexpLevel9
syn region soloSexpLevel9 matchgroup=soloParen9 start="(" matchgroup=soloParen9 end=")" contained contains=@soloTopCluster,soloSexpLevel0

syn region  soloAnonFn  matchgroup=soloParen0 start="#(" matchgroup=soloParen0 end=")"  contains=@soloTopCluster,soloAnonArg,soloSexpLevel0
syn region  soloVector  matchgroup=soloParen0 start="\[" matchgroup=soloParen0 end="\]" contains=@soloTopCluster,soloVarArg,soloSexpLevel0
syn region  soloMap     matchgroup=soloParen0 start="{"  matchgroup=soloParen0 end="}"  contains=@soloTopCluster,soloSexpLevel0
syn region  soloSet     matchgroup=soloParen0 start="#{" matchgroup=soloParen0 end="}"  contains=@soloTopCluster,soloSexpLevel0

syn region  soloPattern start=/L\=\#"/ skip=/\\\\\|\\"/ end=/"/

" FIXME: Matching of 'comment' is broken. It seems we can't nest
" the different highlighting items, when they share the same end
" pattern.
" See also: https://bitbucket.org/kotarak/vimsolo/issue/87/comment-is-highlighted-incorrectly
"
"syn region  soloCommentSexp                          start="("                                       end=")" transparent contained contains=soloCommentSexp
"syn region  soloComment     matchgroup=soloParen0 start="(comment"rs=s+1 matchgroup=soloParen0 end=")"                       contains=soloTopCluster
syn match   soloComment "comment"
syn region  soloComment start="#!" end="\n"
syn match   soloComment "#_"

syn sync fromstart

if version >= 600
	command -nargs=+ HiLink highlight default link <args>
else
	command -nargs=+ HiLink highlight         link <args>
endif

HiLink soloConstant  Constant
HiLink soloBoolean   Boolean
HiLink soloCharacter Character
HiLink soloKeyword   Operator
HiLink soloNumber    Number
HiLink soloString    String
HiLink soloPattern   Constant

HiLink soloVariable  Identifier
HiLink soloCond      Conditional
HiLink soloDefine    Define
HiLink soloException Exception
HiLink soloFunc      Function
HiLink soloMacro     Macro
HiLink soloRepeat    Repeat

HiLink soloQuote     Special
HiLink soloUnquote   Special
HiLink soloDispatch  Special
HiLink soloAnonArg   Special
HiLink soloVarArg    Special
HiLink soloSpecial   Special

HiLink soloComment   Comment
HiLink soloTodo      Todo

HiLink soloError     Error

HiLink soloParen0    Delimiter

let g:vimsolo#ParenRainbowColorsDark = {
    \ '1': 'ctermfg=yellow      guifg=orange1',
    \ '2': 'ctermfg=green       guifg=yellow1',
    \ '3': 'ctermfg=cyan        guifg=greenyellow',
    \ '4': 'ctermfg=magenta     guifg=green1',
    \ '5': 'ctermfg=red         guifg=springgreen1',
    \ '6': 'ctermfg=yellow      guifg=cyan1',
    \ '7': 'ctermfg=green       guifg=slateblue1',
    \ '8': 'ctermfg=cyan        guifg=magenta1',
    \ '9': 'ctermfg=magenta     guifg=purple1'
    \ }

function! VimsoloSetupParenRainbow()
  let colors = g:vimsolo#ParenRainbowColorsDark

  for [level, color] in items(colors)
    execute "highlight soloParen" . level . " " . color
  endfor
endfunction
call VimsoloSetupParenRainbow()
augroup VimsoloSyntax
  au!
  autocmd ColorScheme * if &ft == "solo" | call VimsoloSetupParenRainbow() | endif
augroup END

"	No rainbow
"	HiLink soloParen1 soloParen0
"	HiLink soloParen2 soloParen0
"	HiLink soloParen3 soloParen0
"	HiLink soloParen4 soloParen0
"	HiLink soloParen5 soloParen0
"	HiLink soloParen6 soloParen0
"	HiLink soloParen7 soloParen0
"	HiLink soloParen8 soloParen0
"	HiLink soloParen9 soloParen0

delcommand HiLink

let b:current_syntax = "solo"
