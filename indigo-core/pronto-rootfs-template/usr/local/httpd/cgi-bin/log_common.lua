-- Common log display code

require "web_common"

local content_template = [[
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html lang="en-US" xml:lang="en-US" xmlns="http://www.w3.org/1999/xhtml">

<head>
]]
content_template = content_template .. common_includes
content_template = content_template .. [[
	<script type="text/javascript">
	  $(document).ready(function(){
        $('#__LOGNAME___form').ajaxForm({target: '#__LOGNAME___content'});
	  });
	</script>
</head>
<body>

<div id="__LOGNAME___content">

<form name="input" action="/cgi-bin/__LOGNAME__" method="POST" id="__LOGNAME___form">
<input type="submit" value="Refresh Log" />
<input type="hidden" name="Hidden" value="Clicked" />
                  ]]

content_template = content_template .. "Current time: ".. os.date().. "<br>"

content_template = content_template .. [[
<textarea cols='100' rows='40' readonly>
__LOG_CONTENT__
</textarea>
</form>
</body>
</html>
]]

-- Function to generate log page content.  Passed the name of the
-- log, which must be the script name, and the content to display
function print_log_page_content(name, content) 
   local page_content = string.gsub(content_template, "__LOGNAME__", name)
   page_content = string.gsub(page_content, "__LOG_CONTENT__", content)
   print(page_content)
end

-- Initialize log_dir
log_dir = os.getenv('log_dir')

