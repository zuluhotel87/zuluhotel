use uo;
use os;

include "include/namingbyenchant";
//include "include/attributes";
include "include/spelldata";
include "include/client";
include "include/eventid";
include "include/classes";
include "include/itemutil";
include "include/random";
include "include/yesno";

//Used objects
const RITUAL_ROBE        := 0xba29;
const OAK_STAFF          := 0xba28;
const NORTH_RUNE         := 0xba24;
const MANA_CRYSTAL       := 0xba2c;
const MANA_PILE          := 0x3789;
const WORDS_CAPTOR       := 0xba2b;
const MANA_FLUX          := 0x3778;

//Switchs for processRitual
const UNDO_CIRCLE        := 0x000;
const KEEP_CIRCLE        := 0x001;

//Switchs for targetting
const CAN_BE_EQUIPPED    := 0x000;
const IS_A_WEAPON        := 0x002;
const IS_AN_ARMOR        := 0x004;
const HAVE_HP            := 0x006;
const CAN_BE_CURSED      := 0x000;
const CANNOT_BE_CURSED   := 0x001;


var captor;
var cfg;

function TryToDrawCircle( who )

	var x := who.x;
	var y := who.y;
	var z := who.z;
	var center_y := y + 5;

	if( len(ListItemsNearLocation( x, center_y, z, 6 )) >= 1 )
		SendSysMessage( who, "You don't have the necessary space to draw a circle there." );
		EraseObjProperty( who, "#Casting" );
		return 0;
	endif


	cfg := ReadConfigFile( "rituals" );


	var elem := FindConfigElem( cfg, "DrawCircle" );
	var current := 1;


	var starting_hp := GetHp(who);
	var magic_circle := {};
	var loop;

	var speechs_delay := cfg["General"].DelayBetweenSpeechs;
	var base_light    := cfg["General"].BaseLightLevel;


	//Create the speech captor
	captor := CreateItemAtLocation( x, center_y, z, WORDS_CAPTOR, 1 );
	RegisterForSpeechEvents( captor, 6 );
	EnableEvents( SYSEVENT_SPEECH, 6 );

	SendSysMessage( who, "You can start to draw the circle" );


	while( GetConfigString( elem, "Speech" + current ) )

		var speech := GetConfigString( elem, "Speech" + current );
		var text := GetConfigString( elem, "Print"  + current );
		var x_mod := GetConfigInt(    elem, "Xmod"   + current );
		var y_mod := GetConfigInt(    elem, "Ymod"   + current );
		var facing := GetConfigInt(    elem, "Facing" + current );
		var action := CInt( GetConfigString( elem, "Action" + current ) );
		var appear := CInt( GetConfigString( elem, "Appear" + current ) );
		var anim := CInt( GetConfigString( elem, "Anim"   + current ) );
		var sfx := CInt( GetConfigString( elem, "Sfx"    + current ) );
		var consume := GetConfigStringArray( elem, "Consume" + current );

		//debugging happening here

		sleep( speechs_delay );
		loop := 1;
		var ev;

		while( loop )

			if( who.x != (x + x_mod) || who.y != (y + y_mod) || GetHp(who) < starting_hp )
				DestroyDone( who, magic_circle );
				return 0;
			endif

			ev := os::wait_for_event( 5 );

			if( ev && ev.source == who )
				set_critical(1);
				if( ev.type == SYSEVENT_SPEECH )
					if( ev.text[speech] )
						loop := 0;
					else
						DisableEvents( SYSEVENT_SPEECH );
						DestroyDone( who, magic_circle );
						return 0;
					endif
				endif
				set_critical(0);
			endif

		endwhile

		if( len(consume) )
			set_critical(1);
			if(consume)
			foreach substance in consume
				var params	:= SplitWords( substance );
				var objtype	:= CInt( params[1] );
				if( !objtype )
					objtype := GetObjtypeByName( params[1] );
				endif

				var amount	:= 1;
				if( params[2] )
					amount	:= CInt( params[2] );
				endif

				if( !ConsumeObjType( who, objtype, amount ) )
					SendSysMessage( who, "You miss some substances to pursue the ritual any further." );
					DisableEvents( SYSEVENT_SPEECH );
					DestroyDone( who, magic_circle );
					EraseObjProperty( who, "#Casting" );
					return 0;
				endif
			endforeach
			endif
			set_critical(0);
		endif

		if( text )
			PrintTextAbove( who, text );
		endif

		if( action )
			PerformAction( who, action );
		endif

		if( anim )
			PlayObjectCenteredEffect( who, anim, 0xa, 0xa );
		endif

		if( sfx )
			PlaySoundEffect( who, sfx );
		endif

		if( appear )
			var itemx_mod := GetConfigInt( elem, "Xmod" + (current + 1) );
			var itemy_mod	:= GetConfigInt( elem, "Ymod" + (current + 1) );
			magic_circle.append( CreateItemAtLocation( (x + itemx_mod), (y + itemy_mod), z, appear, 1 ) );
		endif

		foreach mobile in ListMobilesNearLocation( who.x, who.y, who.z, 10 );
			mobile.setlightlevel( base_light * current, 600 );
		endforeach


		current := current + 1;

	endwhile

	PerformAction( who, ANIM_CAST_AREA );
	PlaySoundEffect( who, 0x01fe );
	CreateMagicCircle( x, y, z, magic_circle );

	sleep( 1 );

	CreateCandles( x, y, z, magic_circle );
	MakeTheCircleUnDecayable( magic_circle );

	return magic_circle;

endfunction



function CreateMagicCircle( x, y, z, byref magic_circle )

	magic_circle.append( CreateItemAtLocation( (x + 2), (y + 7), z, 0xba01, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x + 1), (y + 7), z, 0xba02, 1 ) );
	magic_circle.append( CreateItemAtLocation(  x, (y + 7), z, 0xba03, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x - 1), (y + 7), z, 0xba04, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x - 2), (y + 7), z, 0xba05, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x - 2), (y + 6), z, 0xba06, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x - 2), (y + 5), z, 0xba07, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x - 2), (y + 4), z, 0xba08, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x - 2), (y + 3), z, 0xba09, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x - 1), (y + 3), z, 0xba0a, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x - 1), (y + 2), z, 0xba0b, 1 ) );
	magic_circle.append( CreateItemAtLocation(  x, (y + 2), z, 0xba0c, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x + 1), (y + 2), z, 0xba0d, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x + 2), (y + 2), z, 0xba0e, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x + 3), (y + 2), z, 0xba0f, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x + 3), (y + 3), z, 0xba10, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x + 3), (y + 4), z, 0xba11, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x + 3), (y + 5), z, 0xba12, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x + 3), (y + 6), z, 0xba13, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x + 2), (y + 6), z, 0xba14, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x + 1), (y + 6), z, 0xba15, 1 ) );
	magic_circle.append( CreateItemAtLocation(  x, (y + 6), z, 0xba16, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x - 1), (y + 6), z, 0xba17, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x - 1), (y + 5), z, 0xba18, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x - 1), (y + 4), z, 0xba19, 1 ) );
	magic_circle.append( CreateItemAtLocation(  x, (y + 3), z, 0xba1a, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x + 1), (y + 3), z, 0xba1b, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x + 2), (y + 3), z, 0xba1c, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x + 2), (y + 4), z, 0xba1d, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x + 2), (y + 5), z, 0xba1e, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x + 1), (y + 5), z, 0xba1f, 1 ) );
	magic_circle.append( CreateItemAtLocation(  x, (y + 5), z, 0xba20, 1 ) );
	magic_circle.append( CreateItemAtLocation(  x, (y + 4), z, 0xba21, 1 ) );
	magic_circle.append( CreateItemAtLocation( (x + 1), (y + 4), z, 0xba22, 1 ) );

endfunction



function CreateCandles( x, y, z, byref magic_circle )

	magic_circle.append( CreateItemAtLocation(  x, (y + 2), z, 0x1854, 1 ) );
	magic_circle[len(magic_circle)].movable := 0;
	SetName( magic_circle[len(magic_circle)], "Magic Candle" );
	PlaySoundEffect( magic_circle[len(magic_circle)], 0x022b );
	sleep( 1 );

	magic_circle.append( CreateItemAtLocation( (x + 3), (y + 3), z, 0x1854, 1 ) );
	magic_circle[len(magic_circle)].movable := 0;
	SetName( magic_circle[len(magic_circle)], "Magic Candle" );
	PlaySoundEffect( magic_circle[len(magic_circle)], 0x022b );
	sleep( 1 );

	magic_circle.append( CreateItemAtLocation( (x + 2), (y + 6), z, 0x1854, 1 ) );
	magic_circle[len(magic_circle)].movable := 0;
	SetName( magic_circle[len(magic_circle)], "Magic Candle" );
	PlaySoundEffect( magic_circle[len(magic_circle)], 0x022b );
	sleep( 1 );

	magic_circle.append( CreateItemAtLocation( (x - 1), (y + 7), z, 0x1854, 1 ) );
	magic_circle[len(magic_circle)].movable := 0;
	SetName( magic_circle[len(magic_circle)], "Magic Candle" );
	PlaySoundEffect( magic_circle[len(magic_circle)], 0x022b );
	sleep( 1 );

	magic_circle.append( CreateItemAtLocation( (x - 2), (y + 4), z, 0x1854, 1 ) );
	magic_circle[len(magic_circle)].movable := 0;
	SetName( magic_circle[len(magic_circle)], "Magic Candle" );
	PlaySoundEffect( magic_circle[len(magic_circle)], 0x022b );
	sleep( 1 );

endfunction



function DestroyDone( who, magic_circle, item := 0 )

	set_critical(1);
	var amount := len(magic_circle);

	var mana := GetMana(who);

	if( GetObjProperty( magic_circle[1], "PermMagicCircle" ))
		var i;
		for( i := 5; i <= amount; i := i + 1 )
			DestroyItem( magic_circle[i] );
		endfor
	else
		foreach part in magic_circle
			DestroyItem( part );
		endforeach
	endif

	if( item )
		DestroyItem( item );
	endif

	PrintTextAbove( who, "*The mystic energy go out of control and unleash itself upon " + who.name + "!*" );
	PlaySoundEffect( who, SFX_SPELL_LIGHTNING );
	SetMana(who, 0);
	ApplyRawDamage( who, amount * 2 + mana );
	EraseObjProperty( who, "#Casting" );
	DestroyItem( captor );
	set_critical(0);

endfunction



function ProcessRitual( who, item, byref magic_circle, the_ritual, keep_circ := UNDO_CIRCLE )

	var x  := magic_circle[1].x;
	var y  := magic_circle[1].y;
	var z  := magic_circle[1].z;
	var cx := x;
	var cy := y + 5;
	var cz := z;

	var wizards := array( who );
	var manapool := 0;

	if( item )
		MoveItemToLocation( item, cx, cy, cz, MOVEITEM_FORCELOCATION );
		item.movable := 0;
	endif

	SendSysMessage( who, "Do you wish to access some mana crystals' power?" );
	if( YesNo( who, "Use crystals?" ) )
		while( 1 )
			SendSysMessage( who, "Select a mana crystal to access, Hit Esc to cancel." );
			var crystal := Target( who, TGTOPT_CHECK_LOS );

			if( !crystal )
				break;
			elseif( !Accessible( who, crystal ) )
				SendSysMessage( who, "You can't reach that" );
			elseif( crystal.objtype != MANA_CRYSTAL )
				SendSysMessage( who, "You can only use mana crystal" );
			elseif( !GetObjProperty( crystal, "ManaLevel" ) )
				SendSysMessage( who, "This crystal have no mystic energy left to use." );
			else
				manapool := manapool + GetObjProperty( crystal, "ManaLevel" );
				EraseObjProperty( crystal, "ManaLevel" );
			endif
		endwhile
	endif

	SendSysMessage( who, "Do you wish to invite others wizards to your ritual?" );
	if( YesNo( who, "Invite wizards?" ) )
		while( 1 )
			SendSysMessage( who, "Select the wizard you want to invite. Hit Esc to cancel." );
			var wizard := Target( who );

			if( !wizard )
				break;
			elseif( !wizard.acctname )
				SendSysMessage( who, "That isn't a wizard." );
			elseif( !YesNo( wizard, "Agree to join?" ) )
				SendSysMessage( who, "He don't want to join the ritual." );
			elseif( !CheckRitualEquip( wizard ) )
				SendSysMessage( who, "That wizard isn't wearing the required equipment to perform a ritual." );
				SendSysMessage( wizard, "You aren't wearing the required equipment to perform a ritual." );
			else
				wizards.append( wizard );
				SendSysMessage( who, wizard.name + " agreed to join the ritual" );
				SendSysMessage( wizard, "You can go in the Circle" );
			endif
		endwhile
	endif

	sleep( len(wizards) );

	PrintTextAbove( item, "*Every wizard gather in the circle as the magical ritual start*" );

	var elem := FindConfigElem( cfg, the_ritual );
	var amount := 1;
	while( GetConfigString( elem, "Speech" + amount ) )
		amount := amount + 1;
	endwhile

	amount := amount - 1;

	var manapile	:= CreateItemAtLocation( cx, cy, cz+(4*amount), MANA_PILE, 1 );
	SetName( manapile, "Mana Cloud" );
	manapile.movable := 0;

	SetObjProperty( captor, "RitualRunning", 1 );

	var parms := {};
	parms[1] := wizards;
	parms[2] := magic_circle[1];
	parms[3] := captor;

	start_script( ":artificier:wizardpozcheck", parms );



	var current := 1;
	var loop;

	while( GetObjProperty( captor, "RitualRunning" ) && GetConfigString( elem, "Speech" + current ) )

		var speech	:= GetConfigString( elem, "Speech" + current );
		var text	:= GetConfigString( elem, "Print"  + current );
		var appears	:= GetConfigString( elem, "Appear" + current );
		var action	:= CInt( GetConfigString( elem, "Action" + current ) );
		var anim	:= CInt( GetConfigString( elem, "Anim"   + current ) );
		var sfx	:= CInt( GetConfigString( elem, "Sfx"    + current ) );
		var x_mod	:= GetConfigInt( elem, "Xmod" + current );
		var y_mod	:= GetConfigInt( elem, "Ymod" + current );
		var facing	:= GetConfigInt( elem, "Facing" + current );
		var consume	:= GetConfigStringArray( elem, "Consume" + current );

		loop := 1;
		var ev;
		while( loop && GetObjProperty( captor, "RitualRunning" ) )

			ev := os::wait_for_event( 5 );
			if( ev )
				if( ev.source in wizards )
					if( ev.type == SYSEVENT_SPEECH )
						if( ev.text[speech] && !GetObjProperty( ev.source, "SaidFormula" + current ))
							if( x_mod )
								if( ev.source.x != x + x_mod )
									DisableEvents( SYSEVENT_SPEECH );
									DestroyItem( manapile );
									foreach wizard in wizards
										EraseObjProperty( wizard, "SaidFormula" + current );
									endforeach

									EnergyDiruption( wizards, item, magic_circle, manapool );
									EraseObjProperty( who, "#Casting" );
									return 0;
								endif
							endif

							if( y_mod )
								if( ev.source.y != y + y_mod )
									DisableEvents( SYSEVENT_SPEECH );
									DestroyItem( manapile );
									foreach wizard in wizards
										EraseObjProperty( wizard, "SaidFormula" + current );
									endforeach

									EnergyDiruption( wizards, item, magic_circle, manapool );
									EraseObjProperty( who, "#Casting" );
									return 0;
								endif
							endif

							SetObjProperty( ev.source, "SaidFormula" + current, 1 );
							var mana_amount := CInt( GetMana(ev.source) / 4 );
							SetMana(ev.source, GetMana(ev.source.mana) - mana_amount);
							manapool := manapool + mana_amount;
							PlayMovingEffect( ev.source, manapile, MANA_FLUX, 0xa, 0xa );

							if( action )
								PerformAction( ev.source, action );
							endif

							if( text )
								PrintTextAbove( ev.source, text );
							endif

							if( anim )
								PlayObjectCenteredEffect( ev.source, anim, 0xa, 0xa );
							endif

							if( sfx )
								PlaySoundEffect( ev.source, sfx );
							endif

						else

							DisableEvents( SYSEVENT_SPEECH );
							DestroyItem( manapile );
							foreach wizard in wizards
								EraseObjProperty( wizard, "SaidFormula" + current );
							endforeach

							EnergyDiruption( wizards, item, magic_circle, manapool );
							EraseObjProperty( who, "#Casting" );
							return 0;
						endif
					endif
				endif
			endif

			var total := 0;
			foreach wizard in wizards
				if( GetObjProperty( wizard, "SaidFormula" + current ) )
					total := total + 1;
				endif
			endforeach

			if( total == len( wizards ) )

				foreach wizard in wizards
					EraseObjProperty( wizard, "SaidFormula" + current );
					var mana_amount := CInt( GetMana(wizard) / 4 );
					SetMana(wizard, GetMana(wizard) - mana_amount);
					manapool := manapool + mana_amount;
					PlayMovingEffect( wizard, manapile, MANA_FLUX, 0xa, 0xa );
					PerformAction( wizard, ANIM_CAST_DIR );
				endforeach

				if( item )
					PrintTextAbove( item, "*As the power words sequence is finished," );
					PrintTextAbove( item, "The mystic energy channels above the item.*" );
				endif

				if( len(consume) )
					set_critical(1);
					foreach substance in consume
						var params	:= SplitWords( substance );
						var objtype	:= CInt( params[1] );
						if( !objtype )
							objtype := GetObjtypeByName( params[1] );
						endif

						var amount	:= 1;
						if( params[2] )
							amount	:= CInt( params[2] );
						endif

						if( !ConsumeObjType( who, objtype, amount ) )
							SendSysMessage( who, "You miss some substances to pursue the ritual any further." );
							DisableEvents( SYSEVENT_SPEECH );
							DestroyDone( who, magic_circle );
							EraseObjProperty( who, "#Casting" );
							return 0;
						endif
					endforeach
					set_critical(0);
				endif

				if( appears )
					set_critical(1);
					var the_items   := SplitWords( appears );
					var item_x_mods := SplitWords( GetConfigString( elem, "IXmod" + current ) );
					var item_y_mods := SplitWords( GetConfigString( elem, "IYmod" + current ) );

					if( len(the_items) == len(item_x_mods) && len(the_items) == len(item_y_mods) )
						var count;
						for( count := 1; count <= len(the_items); count := count + 1 )
							var to_be_created := CInt( the_items[count] );
							var item_x_mod := CInt( item_x_mods[count] );
							var item_y_mod := CInt( item_y_mods[count] );
							if( !to_be_created )
								to_be_created := GetObjtypeByName( the_items[count] );
							endif

							magic_circle.append( CreateItemAtLocation( x + item_x_mod, y + item_y_mod, z, to_be_created, 1 ) );
							magic_circle[len(magic_circle)].movable := 0;
						endfor
					endif
					set_critical(0);
				endif

				if( item )
					item.movable := 1;
					MoveItemToLocation( item, item.x, item.y, item.z+2, MOVEITEM_FORCELOCATION );
					item.movable := 0;
				endif

				manapile.movable := 1;
				MoveItemToLocation( manapile, manapile.x, manapile.y, manapile.z-2, MOVEITEM_FORCELOCATION );
				manapile.movable := 0;

				loop	:= 0;
				current := current + 1;
				sleep( 2 );

			endif
		endwhile
	endwhile

	if( !GetObjProperty( captor, "RitualRunning" ) )
		DestroyItem( manapile );
		EnergyDiruption( wizards, item, magic_circle, manapool );
		EraseObjProperty( who, "#Casting" );
		return 0;
	endif

	sleep( 5 );

	foreach wizard in wizards
		manapool := manapool + GetMana(wizard);
		SetMana(wizard, 0);
		PlayMovingEffect( wizard, manapile, MANA_FLUX, 0xa, 0xa );
		PerformAction( wizard, ANIM_CAST_DIR );
	endforeach

	PrintTextAbove( who, "*All the mystic energy goes through " + who.name + " as the final ritual process begins*" );
	PlaySoundEffect( who, 0x01fe );

	var times;
	var mana_part := CInt( manapool / 10 );
	for( times := 1; times <= 10; times := times + 1 )
		PlayMovingEffect( manapile, who, MANA_FLUX, 0xa, 0xa );
		SetMana(who, GetMana(who) + mana_part);
		manapool := manapool - mana_part;
		sleepms(500);
	endfor

	DestroyItem( manapile );
	EraseObjProperty( captor, "RitualRunning" );

	//Try to enchant the item
	if( !CheckRitualSkillCheck( wizards, item, magic_circle, the_ritual ) )
		EraseObjProperty( who, "#Casting" );
		return 0;
	endif

	//Try to undo the Circle
	if( len(magic_circle) > 4 )
		if( keep_circ == UNDO_CIRCLE )
			if( !TryToUndoTheCircle( who, magic_circle, item ) )
				EraseObjProperty( who, "#Casting" );
				return 0;
			endif
		else
			var to_be_destroyed := {};
			var index;
			for( index := 5; index <= len( magic_circle ); index := index + 1 )
				to_be_destroyed.append( magic_circle[index] );
			endfor
			DestroyCircle( to_be_destroyed );
		endif
	endif

	foreach mobile in ListMobilesNearLocation( who.x, who.y, who.z, 10 );
		mobile.setlightlevel( 0, 600 );
	endforeach

	if( item )
		ReturnItemToCaster( who, item );
	endif

	foreach wizard in wizards
		SendSysMessage( wizard, "You're exhausted from the ritual" );
		SetMana(wizard, 0);
		SetStamina(wizard, 0);
		SetHp(wizard, 1);
	endforeach

	EndRitual( who, captor );

	return GetRitualPower( wizards, the_ritual );

endfunction


function CheckRitualSkillCheck( wizards, item, magic_circle, spell )

	//Variables declaration and initialisation
	var who	 := wizards[1];
	var conf	 := ReadConfigFile("::spells");
	var elem	 := FindConfigElem(conf, spell);
	var circ	 := GetConfigInt(elem, "Circle");
	conf	 	 := ReadConfigFile("::circles");
	var value	 := conf[circ].PointValue;
	var manacost	 := conf[circ].mana;
	var difficulty := GetSpellDifficulty( spell );
	var min_diff	 := 100 + CInt((circ - 29) * 15);

	if( spell == SPELLID_RITUAL_OF_CONSECRATION || spell == SPELLID_RITUAL_OF_CHANNEL_MANA )
		SetMana(who, GetMana(who) * 2);
	endif

	set_critical(1);
	foreach wizard in wizards
		var magery := CInt( GetEffectiveSkill( wizard, SKILLID_MAGERY ) );
		if( wizard != who )
			magery := CInt( magery / 2 );
			if( GetObjProperty( wizard, CLASSEID_MAGE ) )
				magery := CInt( magery * ClasseBonus( wizard, CLASSEID_MAGE ) );
			endif

		elseif( CInt(GetObjProperty( wizard, CLASSEID_MAGE )) >= 5 )
			magery := CInt( magery * ClasseBonus( wizard, CLASSEID_MAGE ) - CInt( magery * 2) );
		endif

		difficulty := difficulty - magery;
	endforeach

	set_critical(0);

	if( difficulty < min_diff )
		difficulty := min_diff;
	endif

	if( !ConsumeReagents( who, spell ) )
		PlayObjectCenteredEffect(who, FX_SPELL_FIZZLE, SPEED_SPELL_FIZZLE, LOOP_SPELL_FIZZLE );
		PlaySoundEffect(who, SFX_SPELL_FIZZLE);
		SendSysMessage(who, "Insufficient Reagents.");
		DestroyDone( who, magic_circle, item );
		EraseObjProperty( who, "#Casting" );
		EndRitual( who, captor );
		return 0;
	endif

	if( !ConsumeMana( who, spell ) )
		PlayObjectCenteredEffect(who, FX_SPELL_FIZZLE, SPEED_SPELL_FIZZLE, LOOP_SPELL_FIZZLE);
		PlaySoundEffect(who, SFX_SPELL_FIZZLE);
		SendSysMessage(who, "Insufficient Mana.");
		DestroyDone( who, magic_circle, item );
		EraseObjProperty( who, "#Casting" );
		EndRitual( who, captor );
		return 0;
	endif

	if( !CheckSkill( who, SKILLID_MAGERY, difficulty, value ) )
		if( GetObjProperty( who, CLASSEID_MAGE ) )
			if( !CheckSkill( who, SKILLID_MAGERY, difficulty, value ) )
				PlayObjectCenteredEffect(who, FX_SPELL_FIZZLE, SPEED_SPELL_FIZZLE, LOOP_SPELL_FIZZLE );
				PlaySoundEffect(who, SFX_SPELL_FIZZLE);
				SendSysMessage(who, "The spell fizzles");
				DestroyDone( who, magic_circle, item );
				EraseObjProperty( who, "#Casting" );
				EndRitual( who, captor );
				return 0;
			endif
		else
			PlayObjectCenteredEffect(who, FX_SPELL_FIZZLE, SPEED_SPELL_FIZZLE, LOOP_SPELL_FIZZLE );
			PlaySoundEffect(who, SFX_SPELL_FIZZLE);
			SendSysMessage(who, "The spell fizzles");
			DestroyDone( who, magic_circle, item );
			EraseObjProperty( who, "#Casting" );
			EndRitual( who, captor );
			return 0;
		endif
	endif

	sleep( 5 );

	PrintTextAbove( item, "The item absorbed the enchantment successfully!" );
	PlayObjectCenteredEffect( item, FX_SPELL_MARK, SPEED_SPELL_MARK, LOOP_SPELL_MARK );
	PlaySoundEffect( who, SFX_SPELL_MARK );

	foreach wizard in wizards
		AwardFame( wizard, CInt( (difficulty - 90) * 50 ) );
	endforeach

	return 1;

endfunction


function TryToUndoTheCircle( who, magic_circle, item := 0 )

	var cfg	:= ReadConfigFile( "rituals" );
	var elem	:= FindConfigElem( cfg, "UndoCircle" );
	var current	:= 1;

	var x := magic_circle[1].x;
	var y := magic_circle[1].y;
	var z := magic_circle[1].z;
	var starting_hp := GetHp(who);

	var loop;

	var speechs_delay := cfg["General"].DelayBetweenSpeechs;


	SendSysMessage( who, "You must now undo the magic circle to successfully finish the ritual." );

	while( GetConfigString( elem, "Speech" + current ) )

		var speech	:= GetConfigString( elem, "Speech" + current );
		var text	:= GetConfigString( elem, "Print" + current );
		var x_mod	:= GetConfigInt( elem, "Xmod" + current );
		var y_mod	:= GetConfigInt( elem, "Ymod" + current );
		var facing	:= GetConfigInt( elem, "Facing" + current );
		var appear	:= GetConfigInt( elem, "Appear" + current );


		sleep( speechs_delay );
		loop := 1;
		var ev;

		while( loop )

			if( who.x != (x + x_mod) || who.y != (y + y_mod) || GetHp(who) < starting_hp )
				DisableEvents( SYSEVENT_SPEECH );
				DestroyDone( who, magic_circle, item );
				EraseObjProperty( who, "#Casting" );
				EndRitual( who, captor );
				return 0;
			endif

			ev := os::wait_for_event( 5 );

			if( ev && ev.source == who )
				if( ev.type == SYSEVENT_SPEECH )
					if(ev.text[speech] )
						loop := 0;
					else
						DisableEvents( SYSEVENT_SPEECH );
						DestroyDone( who, magic_circle, item );
						EraseObjProperty( who, "#Casting" );
						EndRitual( who, captor );
						return 0;
					endif
				endif
			endif

		endwhile

		if( text )
			PrintTextAbove( who, text );
		endif
		current := current + 1;

	endwhile

	DisableEvents( SYSEVENT_SPEECH );

	DestroyCircle( magic_circle );
	PlaySoundEffect( who, 0x01fe );
	EndRitual( who, captor );

	return 1;

endfunction



function DestroyCircle( magic_circle )

	foreach part in magic_circle
		DestroyItem( part );
	endforeach

endfunction



function CheckRitualEquip( who )

	var staff := GetEquipmentByLayer( who, LAYER_HAND2 );
	var robe  := GetEquipmentByLayer( who, LAYER_ROBE );
	var mount := GetEquipmentByLayer( who, LAYER_MOUNT );

	if( staff.objtype != OAK_STAFF || robe.objtype != RITUAL_ROBE )
		SendSysMessage( who, "You must wear a consecrated ritual robe and young oak staff to perform a ritual." );
		return 0;
	endif

	if( GetObjProperty( staff, "Consecrated" ) != who.serial || GetObjProperty( robe, "Consecrated" ) != who.serial )
		SendSysMessage( who, "Your ritual equipment isn't consecrated" );
		return 0;
	endif

	if( mount )
		SendSysMessage( who, "You can't perform a ritual while mounted." );
		return 0;
	endif

	return 1;

endfunction


function EnergyDiruption( wizards, item, magic_circle, manapool )

	var amount := len( magic_circle );

	foreach wizard in wizards
		manapool := manapool + GetMana(wizard);
		SetMana(wizard, 0);
	endforeach

	DestroyItem( item );

	var dmg := CInt(manapool+(amount*2) / len(wizards) );

	foreach wizard in wizards
		PrintTextAbove( wizard, "*The mystic energy goes out of control and unleashes itself " + wizard.name + "!*" );
		PlaySoundEffect( wizard, SFX_SPELL_LIGHTNING );
		ApplyRawDamage( wizard, dmg );
	endforeach

	if( GetObjProperty( magic_circle[1], "PermMagicCircle" ) )
		var i;
		for( i := 5; i <= amount; i := i + 1 )
			DestroyItem( magic_circle[i] );
		endfor
	else
		foreach part in magic_circle
			DestroyItem( part );
		endforeach
	endif

	EraseObjProperty( wizards[1], "#Casting" );

endfunction


function ReturnItemToCaster( who, item )

	item.movable := 1;
	MoveItemToContainer( item, who.backpack );

endfunction


function MakeTheCircleUnDecayable( magic_circle )

	foreach part in magic_circle
		part.decayAt := 0;
	endforeach

endfunction


function LookForTheCircle( who )

	var is_permanent;
	var magic_circle := {};

	//Check if the caster want to use his personnal circle
	foreach thing in ListItemsAtLocation( who.x, who.y, who.z );
		if( thing.objtype == NORTH_RUNE && GetObjProperty( thing, "PermMagicCircle" ) )
			is_permanent := thing;
			break;
		endif
	endforeach

	//If he have no permanent circle, then he must draw a new one
	if( is_permanent )
		if( GetObjProperty( is_permanent, "PermMagicCircle" ) != who.serial )
			SendSysMessage( who, "You can't perform a ritual by using the circle of someone else." );
			return;
		endif

		magic_circle.append( is_permanent );
		var runes_serials := GetObjProperty( is_permanent, "RunesSerials" );
		if( !runes_serials || len(runes_serials) < 3 )
			SendSysMessage( who, "Your circle was corrupted!" );
			return;
		endif

		foreach serial in runes_serials
			var rune := SystemFindObjectBySerial( CInt(serial) );
			if( !rune )
				SendSysMessage( who, "Your circle was corrupted!" );
				return;
			endif
			magic_circle.append( rune );
		endforeach

		cfg := ReadConfigFile( "rituals" );

		//Create the speech captor
		captor := CreateItemAtLocation( who.x, who.y + 5, who.z, WORDS_CAPTOR, 1 );
		RegisterForSpeechEvents( captor, 6 );
		EnableEvents( SYSEVENT_SPEECH, 6 );

		PrintTextAbove( who, "*The sky darken*" );
		foreach mobile in ListMobilesNearLocation( who.x, who.y, who.z, 10 );
			mobile.setlightlevel( 24, 600 );
		endforeach

		CreateMagicCircle( is_permanent.x, is_permanent.y, is_permanent.z, magic_circle );
		CreateCandles( is_permanent.x, is_permanent.y, is_permanent.z, magic_circle );
		MakeTheCircleUnDecayable( magic_circle );

		return magic_circle;
	else
		var circle := TryToDrawCircle( who );
		if( !circle )
			EndRitual( who, captor );
		endif
		return circle;
	endif

endfunction


function TargetItemToEnchant( who, magic_circle, options, msg )

	while( 1 )
		SendSysMessage( who, msg );
		var item := Target( who, TGTOPT_CHECK_LOS );

		if( !item )
			SendSysMessage( who, "The ritual is aborted" );
			EraseObjProperty( who, "#Casting" );

			if( GetObjProperty( magic_circle[1], "PermMagicCircle" ))
				var i;
				for( i := 5; i <= len(magic_circle); i := i + 1 )
					DestroyItem( magic_circle[i] );
				endfor
			else
				TryToUndoTheCircle( who, magic_circle );
			endif
			return 0;

		elseif( !Accessible( who, item ) )
			SendSysMessage( who, "You can't reach that" );

		elseif( !ReserveItem( item ) )

		elseif( item in ListEquippedItems( who ) )
			SendSysMessage( who, "You must unequip it first." );

		elseif( GetObjProperty( item, "UnEnchantable" ) )
			SendSysMessage( who, "You can't enchant this item." );

		elseif( item.newbie )
			SendSysMessage( who, "You can't enchant newbied items." );

		elseif( item.equipscript == "::skilladvancerequip" && item.unequipscript == "::skilladvancerunequip" )

			case( options )

				0x000:
					return item;
					break;


				0x001:
					var curse := GetObjProperty( item, "Cursed" );
					if( curse == 2 || curse == 3 )
						SendSysMessage( who, "You can't target cursed items." );
					else
						return item;
					endif
					break;


				0x002:
					if( item.skillid )
						return item;
					else
						SendSysMessage( who, "You must target a weapon." );
					endif
					break;


				0x003:
					if( item.skillid )
						var curse := GetObjProperty( item, "Cursed" );
						if( curse == 2 || curse == 3 )
							SendSysMessage( who, "You can't target cursed items." );
						else
							return item;
						endif
					else
						SendSysMessage( who, "You must target a weapon." );
					endif
					break;


				0x004:
					if( item.ar )
						return item;
					else
						SendSysMessage( who, "You must target an armor." );
					endif
					break;


				0x005:
					if( item.ar )
						var curse := GetObjProperty( item, "Cursed" );
						if( curse == 2 || curse == 3 )
							SendSysMessage( who, "You can't target cursed items." );
						else
							return item;
						endif
					else
						SendSysMessage( who, "You must target an armor." );
					endif
					break;


				0x006:
					if( item.hp )
						return item;
					else
						SendSysMessage( who, "You must target an item that have hp." );
					endif
					break;


				0x007:
					if( item.hp )
						var curse := GetObjProperty( item, "Cursed" );
						if( curse == 2 || curse == 3 )
							SendSysMessage( who, "You can't target cursed items." );
						else
							return item;
						endif
					else
						SendSysMessage( who, "You must target an item that have hp." );
					endif
					break;

			endcase

		else
			SendSysMessage( who, "The item must be equippable." );
		endif

	endwhile

endfunction


function GetRitualPower( wizards, ritualid )

	var who    := wizards[1];
	var rand   := CInt( GetEffectiveSkill( who, SKILLID_MAGERY ) / 40 );
	var number := len( wizards );
	var circle := GetCircle( ritualid );
	var power  := RandomInt( rand ) + 1;

	if( number >= 5 )
		power := power + 2;
	elseif( number >= 3 )
		power := power + 1;
	endif

	foreach wizard in wizards
		if( GetObjProperty( wizard, CLASSEID_MAGE ) )
			power := power + CInt( ClasseBonus( wizard, CLASSEID_MAGE ) );
		endif
	endforeach

	power := power + 29 - circle;

	if( power > 6 )
		power := 6;
	elseif( power < 1 )
		power := 1;
	endif

	return power;

endfunction


function TryToPerformRitual( who,
		spellid,
		opt 	:= CAN_BE_EQUIPPED + CAN_BE_CURSED,
		msg 	:= "Choose the item you wish to enchant.",
		keep_circ 	:= UNDO_CIRCLE )


	//Check if the caster have some unwanted equipment
	if( !CheckRitualEquip( who ) )
		EraseObjProperty( who, "#Casting" );
		return 0;
	endif


	//Look for the magic circle
	var magic_circle := LookForTheCircle( who );
	if( !magic_circle )
		EndRitual( who, captor );
		return 0;
	endif

	if( GetObjProperty( magic_circle[1], "PermMagicCircle" ))
		keep_circ := KEEP_CIRCLE;
	endif

	//Get the item to enchant
	var item;
	if( msg != "" )
		item := TargetItemToEnchant( who, magic_circle, opt, msg );
		if( !item )
			EndRitual( who, captor );
			return 0;
		endif
	endif

	if( !item )
		item := 0;
	endif

	//Try to perform the ritual
	var power := ProcessRitual( who, item, magic_circle, spellid, keep_circ );
	if( !power )
		EndRitual( who, captor );
		return 0;
	endif


	EndRitual( who, captor );
	return { item, power };


endfunction



function EndRitual( who, captor )

	EraseObjProperty( who, "#Casting" );
	DisableEvents( SYSEVENT_SPEECH );
	DestroyItem( captor );

endfunction