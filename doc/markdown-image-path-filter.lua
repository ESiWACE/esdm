function dump(o)
   if type(o) == 'table' then
      local s = '{ '
      for k,v in pairs(o) do
         if type(k) ~= 'number' then k = '"'..k..'"' end
         s = s .. '['..k..'] = ' .. dump(v) .. ','
      end
      return s .. '} '
   else
      return tostring(o)
   end
end


function Image (element)  
  print(dump(element.src))
  --print("Change Link: " .. element.target)
  element.src = string.gsub(element.src, "^%.%./", "./")
  return element
end
