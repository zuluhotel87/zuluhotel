<?php

if($_POST['form'] == 1) {
	if(strlen($_POST['email']) < 5) {
		echo "<font color='red' size='3'>Email Error ! </font>";
		die();
	}
	if(strlen($_POST['username']) < 1) {
		echo "<font color='red' size='3'>Username Error ! </font>";
		die();
	}

	$file = "../../Data/accounts.txt";

  $name = $_POST['username'];
  $email = $_POST['email'];
  
  $handle = @fopen($file,r);
  $contents = @fread($handle, filesize($file));

  $str = explode("Account",$contents);
  
  if ($name != "admin") {
    for ($i=1;$i<=count($str);$i++) {
    	if (preg_match("/$name/i",$str[$i]) && preg_match("/$email/i",$str[$i])) {
    		$str2 = explode("\r",$str[$i]);
    			for ($t=1;$t<=count($str2);$t++) {
    				if(preg_match("/Password/i",$str2[$t])) {
    					$str3 = strstr($str2[$t],"Password");
    					$passw = str_replace("Password\t","",$str3);
    						mail($email,"Password","Here comes you ZHFA Information\nAccountname: ".$name."\nPassword: ".$passw."\nIf this is not your account, please remove this email immediatly!","From: andreasgidlund_88@hotmail.com");
								echo "<h3>An Email has been now been sent to your email, containing your Password</h3>";
    					break;
							fclose($handle);
    				}
    			}
    		break;
    	}
			else {
				echo "<h4>No info with that username and email was found</h4>";
			}
    }
  }
}
?>
<form action="data.php" method="post">
  <input type=hidden name=form value=1><br>
  <input type=text name=username><br>
  <input type=text name=email><br>
  <input type=submit value=Submit>
</form>

