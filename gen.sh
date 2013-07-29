cat ../../sql/mdl.h | awk '
BEGIN {
  p=0
}
 
{
  if(p) {
#    if($0=="private:") {
#      print "public:"
#    }else{
#      print $0
#    }
    if ($0 ~ /^ +[A-Za-z_][A-Za-z0-9_]+ \*?m_/ && $1 !="return") {
      gsub("Ticket_list", "MDL_context::Ticket_list", $0) 
      print $0
    }
  }
}

$0=="class MDL_context" {
  p=1
  print "class Hack_MDL_context {"
  print "public:"
}

$0=="};" {
  p=0
}

END {
  print "};"
}
' 
