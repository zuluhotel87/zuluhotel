const STARTHAIR := 0x2030;
const ENDHAIR := 0x2060;

function grabloot()
return;
	if( GetGlobalProperty( "#NoNpcLooting" ) )
		return;
	endif

	var numitems := 0;
	var mypack := 0;
	if (!GetObjProperty(me,"serial"))
		SetObjProperty(me, "serial", me.serial);
	endif

	mypack := me.backpack;
	if( !mypack )
		var bpack := CreateItemAtLocation( me.x, me.y, me.z, 0x0e75, 1 );
		if( bpack )
			if( EquipItem( me, bpack ) )
				mypack := bpack;
			else
				return;
			endif
		else
			bpack := CreateItemAtLocation( me.x, me.y, me.z, 0x0e75, 1 );
			if( bpack )
				if( EquipItem( me, bpack ) )
					mypack := bpack;
				else
					return;
				endif
			else
				return;
			endif
		endif
	endif

	var near_items := ListItemsNearLocation(me.x, me.y, me.z,6);

     foreach item in near_items

        if ((GetObjProperty(item,"houseserial")) || (item.multi) || (item.movable!=0)) return; endif

	if( CheckLineOfSight( me, item ) )

	var sh := GetStandingHeight( CInt(item.x), CInt(item.y), CInt(item.z) );
	if (sh.multi || sh.multi.serial)
		break;
	endif
	var info := CInt(GetMapInfo( CInt(item.x), CInt(item.y) ).landtile);
	if ((info > CInt(0x405)) && (info < CInt(0x456)))
		break;
	endif

	if ( (item.movable) && ((item.objtype < STARTHAIR || item.objtype > ENDHAIR))  )
		RunToIt(me,item);
		if ( Distance(me, item)<2)
			if (playeritem(item, mypack))
			say( "*yoink*" );
			sleep(2);
			return;
			endif
		else
			return;
		endif
	elseif (item.objtype == UOBJ_CORPSE)
	    if (!GetObjProperty(item,"#ignoreit") && !GetObjProperty( item, "nocut" ) )
		var items := { };

		foreach thingie in EnumerateItemsInContainer(item)
		if (    (thingie.objtype < STARTHAIR || thingie.objtype > ENDHAIR)
			&& (!CInt(GetObjProperty(thingie,"#ignoreit"))) )
			items.append( thingie );
		endif
		endforeach

		if ( len(items) )
			RunToIt(me,item);
			if (Distance(me, item) < 2)
				foreach thingie in items
				    if ( playeritem(thingie, mypack) )
				    	var looters := GetObjProperty( item, "LootedBy" );
					if( !looters )
						looters := {};
					endif
				
					if( !(me.name in looters) )
						looters.append( me.name );
						SetObjProperty( item, "LootedBy", looters );
					endif
				
					say( "*yoink*" );
					sleep(2);
					return;
				    else
					SetObjProperty(thingie,"#ignoreit",1);
				    endif
				endforeach
			endif
		else
			SetObjProperty(item,"#ignoreit",1);
		endif

	    endif
	endif

	endif
     endforeach

endfunction

function RunToIt(me,item)

	var numsteps;
	for (numsteps := 0; numsteps < 8; numsteps := numsteps +1)
		if ( Distance(me, item) < 2 )
			return;
		else
			RunToward(item);
		endif
	endfor

endfunction

function playeritem(byref item, byref mypack)

SetObjProperty( item, "dropitem", 1 );

var usescript:=GetObjProperty( item, "old_us");
if(usescript)
	EraseObjProperty( item, "old_us");
	item.usescript := CStr(usescript);
else
	item.usescript := "";
endif

return MoveItemToContainer(item, mypack);

endfunction