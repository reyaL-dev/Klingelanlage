//------------------------------------------------------------------------------
/// @file Klingelanlage.ino
/// @brief      Steuerung der Klingelanlage mit mehreren Eingängen. Zusätzliche
///             Ausgabe an verschiedenen Blinkanlagen.
/// @author     Timm Schütte
/// @author     Till Westphalen
/// @version    2.0.4
/// @date       15. August 2018 - Finalizing
/// @date       27. Juni 2018 	- Entfernen des Interrupts
/// @date       24. Juni 2018 	- Einführung Namenskonvention
/// @date       21. Juni 2018 	- Auslagerung in Header
/// @date       20. Juni 2018 	- Update 2.0
/// @date       14. Dezember 2017 - Entwurf
/// @copyright  GNU Public License.

#include "Klingelanlage.h"				/*!< Variablen-Deklarationen */
#include <Arduino.h>					/*!< Nur für Clang-Completion, wird eh immer eingebunden */
#include "utility.h"

//------------------------------------------------------------------------------
/// @brief      Merker der verschiedenen States.
volatile uint8_t bytLastState = 0;
/// @brief      Timer des Programmes.
volatile structTimer structTimings =
{
	.u_lngLaufzeit = 0,
	.u_lngLeuchtdauer = 0,
	.u_lngEntpreller = 0,
	.bytState = 0,
};

//------------------------------------------------------------------------------
/// @brief      Anfangsroutine bei Start des Programmes Startet den Interrrupt
///             und deaktiviert das Relais.
///
void StartRoutine()
{
	digitalWrite( OUT_TRELAIS, HIGH );
	Serial.println("NEUSTART");
	ResetRoutine();
}


//------------------------------------------------------------------------------
/// @brief      Überprüft, ob Klingel betätigt wurde, und Entprellzeiten
///             eingehalten wurde
///
void KlingelRoutine()
{
	if (bGetState( STATE_KLINGEL_ROUTINE, ADRESS_STATES_GENERIC ))
	{
		if (bGetState( STATE_TIMER_LEUCHTDAUER, ADRESS_STATES_TIMER ))
		{
			if (structTimings.u_lngLeuchtdauer >= CONST_LEUCHTDAUER)
			{
				ResetRoutine();
			}
		}
		else
		{
			SetState( STATE_TIMER_LEUCHTDAUER, ADRESS_STATES_TIMER, true );
			digitalWrite( OUT_TRELAIS, LOW );
		}
	}
	else if (bGetState( STATE_KLINGEL_PUSHED, ADRESS_STATES_GENERIC ))
	{
		if (( CONST_ENTPRELLDAUER < structTimings.u_lngEntpreller ) && ( !digitalRead(IN_KLINGEL)) )
		{
			SetState( STATE_KLINGEL_ROUTINE, ADRESS_STATES_GENERIC, true );
		}
		else
		{
			ResetRoutine();
		}
	}
}


//------------------------------------------------------------------------------
/// @brief      Arduino Setup-Routine. Setzen der PINS & Serieller Debugger.
///
void setup()
{
	pinMode( OUT_BLINKLED,	OUTPUT );
	pinMode( OUT_TESTLED,	OUTPUT );
	pinMode( OUT_TRELAIS,	OUTPUT );
	pinMode( IN_TIMM,		INPUT_PULLUP );
	pinMode( IN_BOBBY,		INPUT_PULLUP );
	pinMode( IN_TILL,		INPUT_PULLUP );
	pinMode( IN_TOBI,		INPUT_PULLUP );
	pinMode( IN_FRANZ,		INPUT_PULLUP );
	pinMode( IN_KLINGEL,	INPUT_PULLUP );
	Serial.begin(115200);				/* für serielle Ausgabe zum debuggen, kann deaktiviert bleiben */
	delay(200);
	StartRoutine();
}

//------------------------------------------------------------------------------
/// @brief      Main-Loop.
///
void loop()
{
	digitalWrite(OUT_TESTLED, digitalRead(IN_TIMM));
	if ((structTimings.u_lngEntpreller > 0) || (structTimings.u_lngLeuchtdauer > 0) )
	{
		Serial.print("\nTimer Entpreller: ");
		Serial.print(structTimings.u_lngEntpreller);
		Serial.print("\nTimer Leuchtdauer: ");
		Serial.print(structTimings.u_lngLeuchtdauer);
	}

	if ( bButtonPushed() )
	{
		if ( !bGetState(STATE_KLINGEL_PUSHED, ADRESS_STATES_GENERIC) )
		{
			// Öffne Tür
			digitalWrite( OUT_TRELAIS, LOW );
			SetState(STATE_KLINGEL_PUSHED, ADRESS_STATES_GENERIC, true);
		}
	}
	else if (bGetState(STATE_KLINGEL_PUSHED, ADRESS_STATES_GENERIC) )
	{
		// Schließe Tür
		digitalWrite( OUT_TRELAIS, HIGH );
		SetState(STATE_KLINGEL_PUSHED, ADRESS_STATES_GENERIC, false);
	}
	//----------------------------------------------------------------------
	// #2 Synchronisation aller Timer.
	UpdateTimings();
	//----------------------------------------------------------------------
	// #3 Klingel-Routine
	KlingelRoutine();
	// #4 Leucht-Routine
	LeuchtRoutine();
}
