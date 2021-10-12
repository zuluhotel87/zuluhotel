//
// Gump Utils
//
// Copyright 2003 Jouni Airaksinen, mintus@bluecave.net
//

use uo;

function SendStringAsHtmlWindow(who, text, title := "Message Of The Day - Zulu Hotel First Age")
  var data := array(
  "page 0",    
//  "nomove",  
  "nodispose", 
//  "noclose", 
  "resizepic 120 100 83 381 228",
  "text 142 108 1152 0",
  "button 427 290 249 248 1 0 0",
  "htmlgump 130 136 361 149 1 1 1"
  );
  var lines := array(
  title,
  text
  );
  
  return SendDialogGump( who, data, lines );
endfunction