-- (c) 2011 Big Switch Networks
-- Parsing unit test

require "ui_utils"
require "parselib"

error_found = 0
verbose = nil
-- To do:  Add more command line parsing if needed
if arg[1] == "-v" then verbose = 1 end

function report_error(where, what)
   print("ERROR in " .. where)
   print(what)
   error_found = 1
end

-- UNIT TESTS for ui_utils.lua

----------------------------------------------------------------
-- Test longest_prefix
----------------------------------------------------------------
if verbose then print("TESTING Test longest_prefix") end

function run_lp(tab, expected)
   local result = longest_prefix(tab)
   if expected ~= result then
      report_error("longest_prefix ", "Expected " .. expected .. ", got " .. 
                   result)
      print("Input: ")
      generic_show(tab)
   end
end

tab1={}
tab1[1]="foob"
tab1[2]="foobar"
tab1[3]="foobaz"

expected = "foob"
run_lp(tab1, expected)

tab1={}
tab1[1]="abc"
tab1[2]="def"

expected = ""
run_lp(tab1, expected)

tab1={}
tab1[1]="abcdef"
tab1[2]="abcdeg"
tab1[3]="abcdeh"
tab1[4]="abcdeh"
tab1[5]="abcdei"

expected = "abcde"
run_lp(tab1, expected)

tab1={}
tab1[1]=""
tab1[2]="d"

expected = ""
run_lp(tab1, expected)

tab1={}
tab1[1]="d"
tab1[2]=""

expected = ""
run_lp(tab1, expected)


----------------------------------------------------------------
-- Test longest_prefix_set
----------------------------------------------------------------
if verbose then print("TESTING Test longest_prefix_set") end

function run_lps(tab, expected)
   local result = longest_prefix_set(tab)
   if expected ~= result then
      report_error("longest_prefix ", "Expected " .. expected .. ", got " .. 
                   result)
      print("Input: ")
      generic_show(tab)
   end
end

tab1={}
tab1["foob"] = true
tab1["foobar"] = true
tab1["foobaz"] = true

expected = "foob"
run_lps(tab1, expected)

tab1={}
tab1["abc"] = true
tab1["def"] = true

expected = ""
run_lps(tab1, expected)

tab1={}
tab1["abcdef"] = true
tab1["abcdeg"] = true
tab1["abcdeh"] = true
tab1["abcdei"] = true

expected = "abcde"
run_lps(tab1, expected)

tab1={}
tab1[""] = true
tab1["d"] = true

expected = ""
run_lps(tab1, expected)

tab1={}
tab1["d"] = true
tab1[""] = true

expected = ""
run_lps(tab1, expected)

----------------------------------------------------------------
-- Test table_value_matches
----------------------------------------------------------------
if verbose then print("TESTING Test table_value_matches") end

function run_tvm(tab, str, e_tab, e_prefix)
   local result_match, result_prefix = table_value_matches(tab, str)

   if not set_compare(result_match, e_tab) or result_prefix ~= e_prefix then
      report_error("table_value_matches error", "match " .. str)
      print("exp_prefix " .. e_prefix .. ". got prefix " .. result_prefix)
      print("input")
      generic_show(tab)
      print("resulting match table")
      generic_show(result_match)
   end
end

tab1={}
tab1[1] = "foob"
tab1[2] = "foobar"
tab1[3] = "foobaz"
tab1[4] = "goo"
tab1[5] = "skaj"
str1 = "fo"

exp_prefix = "foob"
exp_match = {
   foob = true,
   foobar = true,
   foobaz = true,
}
run_tvm(tab1, str1, exp_match, exp_prefix)

str1 = "g"
exp_prefix = "goo"
exp_match = {
   goo = true
}
run_tvm(tab1, str1, exp_match, exp_prefix)

tab1={}
tab1[1] = "abcdef"
tab1[2] = "abcdeg"
tab1[3] = "ljks"
tab1[4] = "89slkj&&lk"
tab1[5] = "abcdeh"
tab1[6] = "abcdei"
str1 = "a"

exp_prefix = "abcde"
exp_match = {
   abcdef = true,
   abcdeg = true,
   abcdeh = true,
   abcdei = true
}
run_tvm(tab1, str1, exp_match, exp_prefix)

str1 = "blah"
exp_match = {}
exp_prefix = ""
run_tvm(tab1, str1, exp_match, exp_prefix)

----------------------------------------------------------------
-- Test table_key_matches
----------------------------------------------------------------
if verbose then print("TESTING Test table_key_matches") end

function run_tkm(tab, str, e_tab, e_prefix)
   local result_match, result_prefix = table_key_matches(tab, str)

   if not set_compare(result_match, e_tab) or result_prefix ~= e_prefix then
      report_error("table_key_matches error", "match " .. str)
      print("exp_prefix " .. e_prefix .. ". got prefix " .. result_prefix)
      print("input")
      generic_show(tab)
      print("resulting match table")
      generic_show(result_match)
   end
end

tab1={}
tab1["foob"] = 1
tab1["foobar"] = 1
tab1["foobaz"] = 1
tab1["goo"] = 1
tab1["skaj"] = 1
str1 = "fo"

exp_prefix = "foob"
exp_match = {
   foob = true,
   foobar = true,
   foobaz = true,
}
run_tkm(tab1, str1, exp_match, exp_prefix)

str1 = "g"
exp_prefix = "goo"
exp_match = {
   goo = true
}
run_tkm(tab1, str1, exp_match, exp_prefix)

tab1={}
tab1["abcdef"] = true
tab1["abcdeg"] = true
tab1["ljks"] = true
tab1["89slkj&&lk"] = true
tab1["abcdeh"] = true
tab1["abcdei"] = true
str1 = "a"

exp_prefix = "abcde"
exp_match = {
   abcdef = true,
   abcdeg = true,
   abcdeh = true,
   abcdei = true
}
run_tkm(tab1, str1, exp_match, exp_prefix)

str1 = "blah"
exp_match = {}
exp_prefix = ""
run_tkm(tab1, str1, exp_match, exp_prefix)

----------------------------------------------------------------
-- parse_params
----------------------------------------------------------------
if verbose then print("TESTING parse_params") end

function bad(line)
   return nil, line
end

-- p means parsed
-- c means continuation
-- e means expected
function test_parse(line, p_tab, e_rc, e_ptab, e_ctab)
   local error = false
   local estr = ""
   local rc, parsed_tab, completions, str

   rc, parsed_tab, completions, str = parse_params(line, p_tab)

   if rc ~= e_rc then
      error = true
      estr = "Return value miscompare; "
   end
   if e_ptab then
      if not table_compare(parsed_tab, e_ptab) then error = true end
      estr = estr .. "Parsed output miscompare; "
   end
   if e_ctab then
      if not set_compare(completions, e_ctab) then error = true end
      estr = estr .. "Continuation miscompare; "
   end

   if error then
      report_error("parse_params line: " .. line, estr)
      print("parse params string: " .. tostring(str))
      print("used p_tab")
      generic_show(p_tab)
      print("Expected rc: " .. e_rc .. ". Got rc: " .. rc)
      if e_ptab then
         print("expected parsed data")
         generic_show(e_ptab)
      end
      if parsed_tab then
         print("received parsed data")
         generic_show(parsed_tab)
      end
      if e_ctab then
         print("expected continuation tab")
         generic_show(e_ctab)
      end
      if completions then
         print("received continuation tab")
         generic_show(completions)
      end
   end
end

p1_tab = {
   key1 = {key="key1", help="key1 help"},
   key2 = {key="key2", parser=parse_yes_no, help="key2 help"},
   key3 = {key="key3", parser=parse_int, help="key3 help"},
   b1 = {key="b1", parser=bad, help="b1 help"},
}

p2_tab = {
   {key="posn1", parser=parse_int, help="posn1 help"},
   key1 = {key="key1", help="key1 help"},
   key2 = {key="key2", parser=parse_yes_no, help="key2 help"},
   key3 = {key="key3", parser=parse_int, help="key3 help"},
   b1 = {key="b1", parser=bad, help="b1 help"},
}

e1_tab = {
   key1 = "abc",
   key2 = 1,
   key3 = 17
}

e2_tab = {
   posn1 = 99,
   key1 = "abc",
   key2 = 1,
   key3 = 17
}

line = " key1=abc key2=true key3=17"
test_parse(line, p1_tab, 0, e1_tab, nil)
line = " key1 = abc key2=yes key3=17"
test_parse(line, p1_tab, 0, e1_tab, nil)

line = "99 key1=abc key2=true key3=17"
test_parse(line, p2_tab, 0, e2_tab, nil)
line = "  99 key1 = abc key2=yes key3=17"
test_parse(line, p2_tab, 0, e2_tab, nil)

line = "99 key1=abc key3=17 unknown"
test_parse(line, p2_tab, -1, nil, nil)

c_tab = { key2 = true, key3 = true}
line = " key1=abc k"
test_parse(line, p1_tab, 1, nil, c_tab)

c_tab = { b1 = true}
line = "b"
test_parse(line, p1_tab, 1, nil, c_tab)

----------------------------------------------------------------
-- Parse line TBD
----------------------------------------------------------------
if verbose then print("TESTING Parse line") end

function test_parse_line(line, cmd_tab, e_rc, e_ptab, e_ctab)
   
end

-- local command_table = {
--    port = {
--       command = "port",
--       parse_tab = port_parse_tab,
--       help = port_help,
--       handler = port_handler
--    }
--    help = {
--       command = "help",
--       parse_tab = help_parse_tab,
--       help = "Show all commands",
--       handler = help_handler
--    }
-- }

----------------------------------------------------------------
-- Parse port spec
----------------------------------------------------------------
if verbose then print("TESTING Parse port spec") end

function test_parser(line, e_tab, compare, parser, arg)
   local o_tab = parser(line, arg)
   if not compare(e_tab, o_tab) then
      report_error("port spec parse " .. line .. ": ", "port set miscompare");
      print("Output: ")
      generic_show(o_tab)
   end
end

e_tab = {1,2,3,4,5}
parser = parse_port_spec
line = "1-5"
test_parser(line, e_tab, set_compare, parser)

line = "1,2,3,4,5"
test_parser(line, e_tab, set_compare, parser)

line = "1,3,5,2,4"
test_parser(line, e_tab, set_compare, parser)

line = "1-3,5,4"
test_parser(line, e_tab, set_compare, parser)

line = "1,2-5"
test_parser(line, e_tab, set_compare, parser)

line = "*"
test_parser(line, e_tab, set_compare, parser, 5)

e_tab = {3,4,5,6,7,8}
line = "3-8"
test_parser(line, e_tab, set_compare, parser)

line = "3,4-7,8"
test_parser(line, e_tab, set_compare, parser)

line = "3-5,6-8"
test_parser(line, e_tab, set_compare, parser)

line = "3-5,7-8,6"
test_parser(line, e_tab, set_compare, parser)

parser = parse_yes_no
exp=0
line="no"
local function eq(a,b) return  a == b end
test_parser(line, exp, eq, parser)

line="0"
test_parser(line, exp, eq, parser)

line="n"
test_parser(line, exp, eq, parser)

line="false"
test_parser(line, exp, eq, parser)

exp=1
line="yes"
local function eq(a,b) return  a == b end
test_parser(line, exp, eq, parser)

line="1"
test_parser(line, exp, eq, parser)

line="true"
test_parser(line, exp, eq, parser)

line="y"
test_parser(line, exp, eq, parser)



----------------------------------------------------------------
-- trim line
----------------------------------------------------------------
if verbose then print("TESTING trim line") end

line = "  abc def  "
exp_result = "abc def"
result = trim_string(line)
if result ~= exp_result then
   report_error("trim line", "wanted >" .. exp_result .. "<; got >" ..
                tostring(result) .. "<")
end

line = "      "
exp_result = ""
result = trim_string(line)
if result ~= exp_result then
   report_error("trim line", "wanted >" .. exp_result .. "<; got >" ..
                tostring(result) .. "<")
end

----------------------------------------------------------------
-- extend token
----------------------------------------------------------------
if verbose then print("TESTING extend token") end

line = "  abc def"
str = "defghi"
exp_result = "  abc defghi"
result = extend_last_token(line, str)
if result ~= exp_result then
   report_error("extend token", "wanted >" .. exp_result .. "<; got >" ..
                tostring(result) .. "<")
end

line = "abc"
str = "abcdef"
exp_result = "abcdef"
result = extend_last_token(line, str)
if result ~= exp_result then
   report_error("extend token", "wanted >" .. exp_result .. "<; got >" ..
                tostring(result) .. "<")
end

----------------------------------------------------------------
-- config parser
----------------------------------------------------------------

----------------------------------------------------------------
-- Report Result
----------------------------------------------------------------

if error_found ~= 0 then
   print("ERROR in ui_utils unit tests")
elseif verbose then
   print("No errors found")
end

os.exit(error_found)
