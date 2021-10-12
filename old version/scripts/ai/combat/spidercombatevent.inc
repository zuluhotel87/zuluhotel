include "include/dotempmods";
//include "include/attributes";
include "include/random";

function process_combat_event(ev)

	send_web(ev.source);
	DoPoison(ev.source);

        if (ev.type == SYSEVENT_DAMAGED)
		if ((GetHp(me) * 100/GetMaxHp(me)) < flee_point)
			EraseObjProperty(me,"#flees");
			flee(ev.source);
		endif
        endif

endfunction

function in_combat_event_loop(opponent)

	send_web(opponent);
	DoPoison(opponent);

endfunction      



function post_combat()

	DisableCombatEvents();
	EnableMainEvents();

	SetWarMode( 0 );
	num_casts := 2;

	sleep(1);
	look_around();
  
endfunction

function prepare_for_fight(opponent)

	DisableMainEvents();
	EnableCombatEvents();

endfunction

function send_web(opponent)

	if (Random(20) != 1)
		return;
	endif

	if (!CheckLineOfSight( me, opponent ))
		return;
	endif

	PrintTextAbove(me, "The spider spits a web!");

	var x := opponent.x;
	var y := opponent.y;
	var z := opponent.z;

	sleep(1);

	CreateItemAtLocation(x, y, z, UOBJ_SPIDERWEB,  1);

	if ( opponent.x == x && opponent.y == y)
		SendSysMessage(opponent, "You are trapped in a spider web!");
		DoTempMod(opponent, "p", 1, 15);
	endif

endfunction

function DoPoison(you)

	if (!GetObjProperty(me,"#lastbreath"))
		SetObjProperty(me,"#lastbreath",ReadGameClock() + 10);
	endif

	if (GetObjProperty(me,"#lastbreath") > ReadGameClock() )
		return;
	endif

	SetObjProperty(me,"#lastbreath",ReadGameClock() + 10);

	if ( (Distance(me,you)<2) && (Random(4)==1) )

		var plvl := Random(GetMaxHp(me)/40)+1;
		SetPoison(you,plvl,"You have been poisoned by " + me.name);

	endif

endfunction