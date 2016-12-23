#!/usr/bin/env python3
import subprocess

def equ(code, value):
    if subprocess.call(['./test_files/test_eq', code, value]):
        print('Failed test "%s"' % code)

def err(code):
    if subprocess.call(['./test_files/test_err', code]):
        print('Failed test "%s" (expected error)' % code)

def test_seq_f(f_name):
    """Test a unary function expecting a string or list as its argument."""
    err('(%s)' % f_name)
    err('(%s 10)' % f_name)
    err('(%s [] [])' % f_name)

# addition
equ('(+ 1 1)', '2')
equ('(+ 1 2 3 4 5)', '15')
equ('(+ 1 2 3.0 4 5)', '15.0')
err('(+)')
err('(+ 1)')
err('(+ 1 [])')
# subtraction and negation
equ('(- 10)', '-10')
equ('(- 10 3)', '7')
equ('(- 10 3.0)', '7.0')
equ('(- 10 8 2 3)', '-3')
err('(-)')
err('(- [])')
err('(- 1 [])')
# multiplication
equ('(* 21 2)', '42')
equ('(* 3.2 7.4)', '23.680000000000003')
equ('(* 1 2 3 4 5 6)', '720')
err('(*)')
err('(* 1)')
err('(* 1 [])')
# real division
equ('(/ 10 2)', '5.0')
equ('(/ -72 2.5)', '-28.8')
equ('(/ 0 42)', '0.0')
err('(/)')
err('(/ 10)')
err('(/ 10 \'abc\')')
err('(/ 10 0)')
err('(/ 10 23 0)')
# floor division
equ('(// 10 3)', '3')
equ('(// 50 11 2)', '2')
equ('(// 0 42)', '0')
err('(// 10 3.0)')
err('(//)')
err('(// 10)')
err('(// 10 "abc")')
err('(// 10 0)')
# remainder
equ('(% 73 2)', '1')
equ('(% 67 7)', '4')
equ('(% 0 10)', '0')
err('(% 42 4.7)')
err('(%)')
err('(% 10)')
err('(% 10 "abc")')
err('(% 10 0)')
# numeric equality
equ('(= 1 1)', 'true')
equ('(= 1 1.0)', 'true')
equ('(= 1.0 1)', 'true')
equ('(= 1  0.999)', 'false')
equ('(= 10 "10")', 'false')
# string equality
equ('(= "abc" "abc")', 'true')
equ('(= "" "")', 'true')
equ('(= "hello" "hallo")', 'false')
equ('(= "short" "a longer string")', 'false')
# list equality
equ('(= [1 2 3] [1 2 3])', 'true')
equ('(= [1 2 3] [1.0 2.0 3.0])', 'true')
equ('(= [1 2 3] [1 2 3 4])', 'false')
equ('(= [] [])', 'true')
equ('(= ["a string" 10 true] ["a string" 10 true])', 'true')
# empty?
equ('(empty? [])', 'true')
equ('(empty? [1 2 3 4 5 6])', 'false')
err('(empty?)')
err('(empty? 10)')
# len
equ('(len [])', '0')
equ('(len [1 2 3 4 5 6])', '6')
equ('(len "abc")', '3')
equ('(len "")', '0')
err('(len)')
err('(len 10)')
err('(len [1 2] [3 4])')
# head
equ('(head [1 2 3 4 5 6])', '1')
equ('(head "abcdef")', '"a"')
err('(head [])')
err('(head "")')
test_seq_f('head')
# tail
equ('(tail [1 2 3 4 5 6])', '[2 3 4 5 6]')
equ('(tail "abcdef")', '"bcdef"')
equ('(tail [])', '[]')
equ('(tail "")', '""')
test_seq_f('tail')
# last
equ('(last [1 2 3 4 5 6])', '6')
equ('(last "abcdef")', '"f"')
err('(last [])')
err('(last "")')
test_seq_f('last')
# init
equ('(init [1 2 3 4 5 6])', '[1 2 3 4 5]')
equ('(init "abcdef")', '"abcde"')
equ('(init [])', '[]')
equ('(init "")', '""')
test_seq_f('init')
# prepend
equ('(prepend 0 [1 2 3 4 5 6])', '[0 1 2 3 4 5 6]')
equ('(prepend 0 [])', '[0]')
# append
equ('(append [1 2 3 4 5 6] 7)', '[1 2 3 4 5 6 7]')
equ('(append [] 0)', '[0]')
err('(append "abcde" "f")')
# concat
equ('(concat [1 2 3] [4 5 6] [7 8 9])', '[1 2 3 4 5 6 7 8 9]')
equ('(concat ["a" true [-17.5 []]] [[42]])', '["a" true [-17.5 []] [42]]')
equ('(concat "ya pomnyu" " chudnoye mgnovenye")', '"ya pomnyu chudnoye mgnovenye"')
equ('(concat [] [])', '[]')
equ('(concat "" "")', '""')
equ('(concat [] [1 2 3])', '[1 2 3]')
# get
equ('(get ["I" "met" "a" "traveller" "from" "an" "antique" "land"] 2)', '"a"')
equ('(get "I met a traveller from an antique land" 6)', '"a"')
equ('(get "abc" 0)', '"a"')
equ('(get "abc" 2)', '"c"')
err('(get "abc" 3)')
err('(get "abc" -1)')
# slice
equ('(slice "no second Troy" 3 9)', '"second"')
equ('(slice [1 2 3 4 5 6 7 8 9] 3 6)', '[4 5 6]')
equ('(slice "abcdefg" 0 4)', '"abcd"')
equ('(slice "abcdefg" 3 7)', '"defg"')
equ('(slice [] 0 0)', '[]')
equ('(slice "" 0 0)', '""')
err('(slice [1 2 3] 2 7)')
err('(slice "abc" -1 2)')
err('(slice "abc" 2 7)')
err('(slice [1 2 3] -1 2)')
err('(slice [] 0 1)')
err('(slice "" 0 1)')
# take
equ('(take [3 1 4 1 5 9] 3)', '[3 1 4]')
equ('(take [1 2 3 4 5] 0)', '[]')
equ('(take "Gazing up into the darkness" 6)', '"Gazing"')
equ('(take "I saw myself as a creature" 0)', '""')
# drop
equ('(drop [3 1 4 1 5 9] 3)', '[1 5 9]')
equ('(drop [1 2 3 4 5] 0)', '[1 2 3 4 5]')
equ('(drop "Death be not proud" 13)', '"proud"')
equ('(drop "though some have called thee" 0)', '"though some have called thee"')
