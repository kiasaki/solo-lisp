" Vim syntax file
" Language:     Solo
" Maintainer:   Frederic Gingras <frederic@gingras.cc>
" Last change:  06 Jul 2015

if exists("b:current_syntax")
  finish
endif

setlocal iskeyword=33,36-38,42,43,45-47,48-57,60-64,@,91-94,_,123-126

syntax case match

syntax match soloSymbol "\<\k\+\>"

syntax keyword soloBuiltin if function new try catch finally do
syntax keyword soloBuiltin instanceof typeof void throw
syntax keyword soloBuiltin def set! get
syntax keyword soloBuiltin + - * / %
syntax keyword soloBuiltin "< <= > >= || && == === != !=="
syntax keyword soloBuiltin null? true? false?
syntax keyword soloBuiltin undefined? boolean? number? string? object? array? function?

syntax match soloStringEscape "\v\\%([\\btnfr"]|u\x{4}|[0-3]\o{2}|\o{1,2})" contained
syntax region soloString start=/"/ skip=/\\\\\|\\"/ end=/"/ contains=soloStringEscape,@Spell

syntax match soloNumber "\v<[-+]?%(0\o*|0x\x+|[1-9]\d*)N?>"
syntax match soloNumber "\v<[-+]?%(0|[1-9]\d*|%(0|[1-9]\d*)\.\d*)%(M|[eE][-+]?\d+)?>"
syntax match soloNumber "\v<[-+]?%(0|[1-9]\d*)/%(0|[1-9]\d*)>"

syntax match soloVarArg "\."

syntax keyword soloBoolean null true false

syntax match soloComment ";.*$" contains=soloCommentTodo,@Spell
syntax keyword soloCommentTodo contained FIXME TODO XXX FIXME: TODO: XXX:

syntax cluster soloTop contains=@Spell,soloComment,soloBoolean,soloVarArg,soloNumber,soloString,soloStringEscape,soloBuiltin,soloSymbol

syntax region soloSexp   matchgroup=soloParen start="("  matchgroup=soloParen end=")" contains=@soloTop fold
syntax region soloVector matchgroup=soloParen start="\[" matchgroup=soloParen end="]" contains=@soloTop fold
syntax region soloMap    matchgroup=soloParen start="{"  matchgroup=soloParen end="}" contains=@soloTop fold

syntax match soloBracketError "]\|}\|)"

syntax sync fromstart

highlight default link soloSymbol       Identifier
highlight default link soloBuiltin      Keyword
highlight default link soloString       String
highlight default link soloStringEscape Character
highlight default link soloNumber       Number
highlight default link soloVarArg       Special
highlight default link soloBoolean      Boolean

highlight default link soloComment      Comment
highlight default link soloCommentTodo  Todo

highlight default link soloParen        Delimiter
"highlight default link soloBracketError Error

let b:current_syntax = "solo"
