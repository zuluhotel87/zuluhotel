use uo;
use os;

const DMGID_FIRE	:= 0x0001;
const DMGID_AIR		:= 0x0002;
const DMGID_EARTH	:= 0x0004;
const DMGID_WATER	:= 0x0008;
const DMGID_NECRO	:= 0x0010;
const DMGID_HOLY	:= 0x0020;
const DMGID_POISON	:= 0x0040;
const DMGID_ACID	:= 0x0080;
const DMGID_PHYSICAL	:= 0x0100;
const DMGID_MAGIC	:= 0x0200;
const DMGID_ASTRAL	:= 0x0400;
const DMGID_NO_RESIST	:= 0x0800;

include "include/constants";
//include "include/attributes";
include "include/areas";
include "../../pkg/opt/karmafame/karmafame";
include "include/mobileutil";
include "include/astralfights";

const MAX_SERIAL_IN_HITLIST := 5;

function ApplyTheDamage( who, by_who, dmg, attack_type := DMGID_PHYSICAL )

	dmg := CInt(dmg);
	if( !who.IsA(POLCLASS_MOBILE) )
		return 0;
	elseif( dmg <= 0 )
		return 0;
	elseif( who.dead )
		return 0;
	elseif( IsImmunedFromThisDamageType( who, dmg, attack_type ) )
		return 0;
	/*elseif( IsInArea( who, AREAID_CRAFTMINE ) ) //safe zone
	    if (who.isA(POLCLASS_NPC) || by_who.isA(POLCLASS_NPC))
		var master:=GetObjProperty(who, "master");
		if(master)
		SetObjProperty( who, "guardkill", 1);
		ApplyRawDamage( who, CInt(GetHp(who)+3) );
		return;
else		

		SendSysMessage(by_who,"We wont tolerate this foolishness...");
		
 			 //NPC THING
			SetObjProperty( who, "guardkill", 1);
			ApplyRawDamage( who, CInt(GetHp(who)+3) );
		
		SetObjProperty( by_who, "guardkill", 1); 
		ApplyRawDamage(by_who, CInt(GetHp(by_who)+3) );
		return;
	endif
	endif               
 		SetObjProperty(by_who,"prohibitedspar",ReadGameClock());
		start_script(":zuluitems:dontspar", by_who);
		return 0;
	elseif( IsInArea( who, AREAID_TRINSIC ) ) //safe zone
		if (who.isA(POLCLASS_NPC) || by_who.isA(POLCLASS_NPC))
	var master:=GetObjProperty(who, "master");

		if(master)
		SetObjProperty( who, "guardkill", 1);
		ApplyRawDamage( who, CInt(GetHp(who)+3) );
		return;
else		
		SendSysMessage(by_who,"We wont tolerate this foolishness...");
		
 			//NPC THING
			SetObjProperty( who, "guardkill", 1);
			ApplyRawDamage( who, CInt(GetHp(who)+3) );
		
		SetObjProperty( by_who, "guardkill", 1); 
		ApplyRawDamage(by_who, CInt(GetHp(by_who)+3) );
		return;
	endif
	endif                
 		SetObjProperty(by_who,"prohibitedspar",ReadGameClock());
		start_script(":zuluitems:dontspar", by_who);
*/		return 0;
	endif

	var factor := 1.0;
	if( by_who.IsA(POLCLASS_NPC) )
		var master := FindMaster(by_who);
		if( master )
			by_who := master;
			factor := factor * VALUE_MULTIPLIER_FOR_TAMED;
		endif
	endif

	if( dmg < GetHp(who) )
		factor := factor*VALUE_MULTIPLIER_FOR_ALLIES;
	else
		if( GetObjProperty(who,PROPID_MOBILE_RISE) )
			if( TryToInflictFinalDeath( who, attack_type ) )
				SetObjProperty( who, PROPID_MOBILE_KILLED_BY, by_who.name );
			else
				factor := factor * VALUE_MULTIPLIER_FOR_ALLIES;
			endif
		else
			SetObjProperty( who, PROPID_MOBILE_KILLED_BY, by_who.name );
		endif
	endif


	if( !by_who )
		return ApplyRawDamage( who, dmg );
	elseif( !by_who.IsA(POLCLASS_MOBILE) )
		return ApplyRawDamage( who, dmg );
	elseif( by_who == who )
		return ApplyRawDamage( who, dmg );
	endif

	SetScriptController(by_who);

	if( !by_who.IsA(POLCLASS_NPC) )
		if( !by_who.cmdlevel )
			// The defender is a npc
			if( who.IsA(POLCLASS_NPC) )
				// Add to hitlist only if the target a) has no master, or, b) has a master that is a criminal/has lower karma
//				if (who.master)
//					if( ((GetKarma(who) < GetKarma(by_who)) || (who.master.criminal && !by_who.criminal) ) )
						GenerateHitlist( who, by_who.serial, factor );
//					endif
//				endif
			else
				var guild1 := GetObjProperty( by_who, "guild_id" );
				if( guild1 )
					var guild2 := GetObjProperty( who, "guild_id" );
					if( guild2 )
						if( guild1 == guild2 )
							return ApplyRawDamage( who, dmg );
						else
							guild1 := FindGuild( guild1 );
							guild2 := FindGuild( guild2 );
							if( guild1.IsEnemyGuild(guild2) )
								return ApplyRawDamage( who, dmg );
							elseif( guild1.IsAllyGuild(guild2) )
								return ApplyRawDamage( who, dmg );
							endif
						endif
					endif
				endif
			
				var defending;
				foreach entry in (by_who.reportables)
					if( who.serial == entry.serial )
						// The attacker is defending himself
						defending := 1;
					endif
				endforeach
			
				if( defending )
					if( (GetKarma(who) < GetKarma(by_who))  || (who.criminal && !by_who.criminal))
						// Add to hitlist only if the target have less karma
						GenerateHitlist( who, by_who.serial, factor );
					endif
				else
					// The attacker is the agressor
					GenerateHitlist( who, by_who.serial, factor );
				endif
			endif
		endif
	endif

	return ApplyRawDamage( who, dmg );

endfunction


function ApplyTheAstralDamage( who, by_who, dmg, attack_type := DMGID_ASTRAL )

	dmg := CInt(dmg);
	if( dmg <= 0 )
		return 0;
	elseif( who.dead )
		return 0;
	elseif( IsImmunedFromThisDamageType( who, dmg, attack_type ) )
		return 0;
	endif

	var factor := 1.0;
	var master := FindMaster(by_who);
	if( master )
		by_who := master;
		factor := factor * VALUE_MULTIPLIER_FOR_TAMED;
	endif

	if( dmg <= GetMana(who) )
		factor := factor * VALUE_MULTIPLIER_FOR_ALLIES;
		SetMana(who, GetMana(who) - dmg);
	else
		dmg := dmg - GetMana(who);
		SetMana(who, 0);
	
		if( dmg <= GetStamina(who) )
			factor := factor * VALUE_MULTIPLIER_FOR_ALLIES;
			SetStamina(who, GetStamina(who) - dmg);
		else
			if( GetObjProperty( who, PROPID_MOBILE_RISE ) )
				TryToInflictFinalDeath( who, attack_type );
			endif
	
			SetStamina(who, 0);
			SetAstralIncapacity( who );
		endif
	endif

	if( !by_who )
		return 1;
	elseif( by_who.IsA(POLCLASS_NPC) )
		return 1;
	elseif( by_who.cmdlevel )
		return 1;
	else
		if( IsSummoned(who) || IsAnimated(who) || IsCharmed(who) )
			master := FindMaster(who);
			if( master )
				GenerateHitlist( master, by_who.serial, factor );
			else
				GenerateHitlist( who, by_who.serial, factor );
			endif
		else
			GenerateHitlist( who, by_who.serial, factor );
		endif

		return 1;
	endif

endfunction


function GenerateHitlist( who, serial, factor )

	var hitlist := GetObjProperty( who, PROPID_MOBILE_HITLIST );
	if( !hitlist )
		hitlist := array;
	endif

	if( len(hitlist) > MAX_SERIAL_IN_HITLIST )
		ReplaceOldestInHitlist( hitlist, serial, factor );
	else
		AddSerialToHitlist( hitlist, serial, factor );
	endif

	SetObjProperty( who, PROPID_MOBILE_HITLIST, hitlist );

endfunction


function ReplaceOldestInHitlist( byref hitlist, serial, factor )

	var min_timer	:= ReadGameClock();
	var min_pos	:= 0;
	var current;

	for( current := 1; current <= len(hitlist); current := current + 1 )
		if( hitlist[current][1] == serial )
			hitlist[current][3] := ReadGameClock();
			if( hitlist[current][2] < factor )
				hitlist[current][2] := factor;
			endif
			return hitlist;
		
		elseif( hitlist[current][3] < min_timer )
			min_timer := hitlist[current][3];
			min_pos   := current;
		endif
	endfor

	if( min_pos )
		hitlist[min_pos][1]	:= serial;
		hitlist[min_pos][2]	:= factor;
		hitlist[min_pos][3]	:= ReadGameClock();
	endif

	return 1;

endfunction

		
function AddSerialToHitlist( byref hitlist, serial, factor )

	foreach arr in hitlist
		if( arr[1] == serial )
			arr[3] := ReadGameClock();
			if( arr[2] < factor )
				arr[2] := factor;
			endif
			return 1;
		endif
	endforeach

	var add	:= { serial, factor, ReadGameClock() };
	return hitlist.append(add);

endfunction


function IsImmunedFromThisDamageType( who, byref dmg, attack_type )

	var immunities := GetObjProperty( who, PROPID_MOBILE_ATTACK_TYPE_IMMUNITIES );
	if( !immunities )
		return 0;
	endif

	if( attack_type & DMGID_NO_RESIST )
		return 0;
	endif

	var attack_amount	:= 0;
	var is_immuned_amount	:= 0;     
	foreach bit in { DMGID_FIRE, DMGID_AIR, DMGID_EARTH, DMGID_WATER, DMGID_NECRO, DMGID_HOLY, DMGID_POISON, DMGID_ACID, DMGID_PHYSICAL, DMGID_MAGIC, DMGID_ASTRAL }
		if( attack_type & bit )
			attack_amount := attack_amount + 1;
			if( immunities & bit )
				is_immuned_amount := is_immuned_amount + 1;
			endif
		endif
	endforeach

	if( !attack_amount || attack_amount == is_immuned_amount )
		return 1;
	else
		dmg := CInt( dmg * CInt(attack_amount-is_immuned_amount) / attack_amount );
		return 0;
	endif

endfunction


function TryToInflictFinalDeath( who, attack_type )

	var final_death := GetObjProperty(who, PROPID_MOBILE_FINAL_DEATH);
	if( !final_death )
		return 0;
	endif

	final_death := CInt(final_death);
	foreach bit in { DMGID_FIRE, DMGID_AIR, DMGID_EARTH, DMGID_WATER, DMGID_NECRO, DMGID_HOLY, DMGID_POISON, DMGID_ACID, DMGID_PHYSICAL, DMGID_MAGIC, DMGID_ASTRAL }
		if( final_death & bit )
			if( !(attack_type & bit) )
				return 0;
			endif
		endif
	endforeach

	return 1;

endfunction


function FindMaster( byref mobile )

	var serial := CInt( GetObjProperty( mobile, PROPID_MOBILE_MASTER_SERIAL ) );
	if( !serial )
		return 0;
	endif

	return FindMobile( serial );

endfunction


function FindMobile( byref serial )

	if( !serial )
		return 0;
	endif

	var mobile := SystemFindObjectBySerial( serial );

	if( !mobile )
		mobile := SystemFindObjectBySerial( serial, SYSFIND_SEARCH_OFFLINE_MOBILES );
	endif

	return mobile;

endfunction


function IsAnimated( byref mobile )

	return GetObjProperty( mobile, PROPID_MOBILE_ANIMATED );

endfunction


function IsCharmed( byref mobile )

	return GetObjProperty( mobile, PROPID_MOBILE_CHARMED );

endfunction


function IsSummoned( byref mobile )

	return GetObjProperty( mobile, PROPID_MOBILE_SUMMONED );

endfunction

/*include "include/astralfights";
include "include/dotempmods";
include "../../pkg/opt/karmafame/karmafame";
include "include/bitwise";

const MAX_SERIAL_IN_HITLIST := 5;
const DMDID_FIRE	:= 0x001;
const DMGID_AIR		:= 0x002;
const DMGID_EARTH	:= 0x004;
const DMGID_WATER	:= 0x008;
const DMGID_ACID	:= 0x010;
const DMGID_NECRO	:= 0x020;
const DMGID_HOLY	:= 0x040;
const DMGID_PHYSICAL	:= 0x100;
const DMGID_MAGIC	:= 0x200;
const DMGID_ASTRAL	:= 0x400;


function ApplyTheDamage( who , by_who , dmg , attack_type := DMGID_PHYSICAL )

	dmg := Cint(dmg);
	if( dmg <= 0 )
		return 0;
	elseif( who.dead )
		return 0;
	elseif( IsImmunedFromThisDamageType( who, attack_type ) )
		return 0;
	endif

	var factor := 1.0;
	var attacker_serial;

	var m_serial := GetObjProperty( by_who , "master" );
	var by_who_name := by_who.name;

	if( m_serial )
		attacker_serial := m_serial;
		factor := factor * VALUE_MULTIPLIER_FOR_TAMED;

	elseif( by_who.npctemplate )
		by_who := 0;
	elseif( by_who = who )
		by_who := 0;
	else
		attacker_serial := by_who.serial;
	endif

	if( dmg < who.hp )
		factor := factor * VALUE_MULTIPLIER_FOR_ALLIES;
	else
		if( GetObjProperty( who , "rise" ) )
			TryToInflictFinalDeath( who, attack_type );
		endif

		if( m_serial )
			var themaster := SystemFindObjectBySerial( cint(m_serial) );
			if( !themaster )
				themaster := SystemFindObjectBySerial( cint(m_serial) , SYSFIND_SEARCH_OFFLINE_MOBILES );
			endif

			if( themaster )
				SetObjProperty( who , "KilledBy" , themaster.name );
			else
				SetObjProperty( who , "KilledBy" , by_who_name );
			endif
		else
			SetObjProperty( who , "KilledBy" , by_who_name );
		endif
	endif

	var list_of_who := who;
	if( GetObjProperty( who , "summoned" ) or GetObjProperty( who , "animated" ) )
		var m_serial2 := Cint( GetObjProperty( who , "master" ) );
		if( !m_serial2 )
			by_who:=0;
		else
		var master	:= SystemFindObjectBySerial( cint(m_serial2) );
		if(master) list_of_who := master; endif
		endif
	endif

	if( by_who )
		GenerateHitlist( list_of_who , attacker_serial , factor );
	endif

	ApplyRawDamage( who , dmg );

endfunction


function ApplyTheAstralDamage( who , by_who , dmg , attack_type := DMGID_ASTRAL )

	if( !dmg )
		return;
	endif

	var factor := 1;
	var attacker_serial;

	var m_serial := GetObjProperty( by_who , "master" );
	var by_who_name := by_who.name;

	if( m_serial )
		attacker_serial := m_serial;
		factor := factor * VALUE_MULTIPLIER_FOR_TAMED;

	elseif( by_who.npctemplate )
		by_who := 0;
	elseif( by_who = who )
		by_who := 0;
	else
		attacker_serial := by_who.serial;
	endif

	if( IsImmunedFromThisDamageType( who, attack_type ) )
		SendSysMessage( by_who , "This creature is immuned to this attack type." );
		return;
	endif

	if( dmg < who.mana )
		factor	 := factor * VALUE_MULTIPLIER_FOR_ALLIES;
		who.mana := who.mana - dmg;
	else
		dmg  := dmg - who.mana;
		who.mana	:= 0;

		if( dmg < who.stamina )
			factor := factor * VALUE_MULTIPLIER_FOR_ALLIES;
			who.stamina := who.stamina - dmg;
		else
			if( GetObjProperty( who , "rise" ) )
				TryToInflictFinalDeath( who, attack_type );
			endif

			who.stamina := 0;
			SetAstralIncapacity( who );
		endif
	endif

	var list_of_who := who;
	if( GetObjProperty( who , "summoned" ) or GetObjProperty( who , "animated" ) )
		var m_serial2 := Cint( GetObjProperty( who , "master" ) );
		if( !m_serial2 )
			by_who:=0;
		else
		var master	:= SystemFindObjectBySerial( cint(m_serial2) );
		if(master) list_of_who := master; endif
		endif
	endif

	if( by_who )
		GenerateHitlist( list_of_who , attacker_serial , factor );
	endif

endfunction


function GenerateHitlist( who , serial , factor )

	var hitlist := GetObjProperty( who , "Hitlist" );
	if( !hitlist )
		hitlist := {};
	endif

	if( len(hitlist) > MAX_SERIAL_IN_HITLIST )
		hitlist := ReplaceOldestInHitlist( hitlist , serial , factor );
	else
		hitlist := AddSerialToHitlist( hitlist , serial , factor );
	endif

	SetObjProperty( who , "Hitlist" , hitlist );

endfunction


function ReplaceOldestInHitlist( hitlist , serial , factor )

	var min_timer := ReadGameClock();
	var min_pos	:= 0;
	var current;

	for( current := 1 ; current <= len(hitlist) ; current := current + 1 )
		if( hitlist[current][1] = serial )
			hitlist[current][2] := ReadGameClock();
			if( hitlist[current][3] < factor )
				hitlist[current][3] := factor;
			endif
			return hitlist;

		elseif( hitlist[current][2] < min_timer )
			min_timer := hitlist[current][2];
			min_pos   := current;
		endif
	endfor

	if( min_pos )
		hitlist[min_pos][1] := serial;
		hitlist[min_pos][2] := ReadGameClock();
		hitlist[min_pos][3] := factor;
	endif

	return hitlist;

endfunction


function AddSerialToHitlist( hitlist , serial , factor )

	foreach arr in hitlist
		if( arr[1] = serial )
			arr[2] := ReadGameClock();
			if( arr[3] < factor )
				arr[3] := factor;
			endif
			return hitlist;
		endif
	endforeach

	var add := { serial , ReadGameClock() , factor };
	hitlist.append( add );

	return hitlist;

endfunction


function IsImmunedFromThisDamageType( who, attack_type )

	var immunities := GetObjProperty( who , "AttackTypeImmunity" );
	if( !immunities )
		return 0;
	endif

	var attack_main_type	 := Cint( attack_type / 128 );
	var immunity_main_type := Cint( immunities  / 128 );
	if( attack_main_type )
		if( BitAnd(attack_main_type, immunity_main_type) )
			attack_type := attack_type - (attack_main_type * 128);
			immunities  := immunities - (immunity_main_type * 128);
			if( BitImp(attack_type, immunities) = 0xff )
				return 1;
			else
				return 0;
			endif
		else
			return 0;
		endif
	else
		immunities  := immunities - (immunity_main_type * 128);
		if( BitImp(attack_type, immunities) = 0xff )
			return 1;
		else
			return 0;
		endif
	endif

	return 0;

endfunction


function TryToInflictFinalDeath( who, attack_type )

	var final_death := Cint(GetObjProperty( who , "FinalDeath" ));
	if( !final_death )
		return;
	endif

	if( BitAnd(attack_type, final_death) )
		EraseObjProperty( who , "rise" );
	endif

endfunction*/