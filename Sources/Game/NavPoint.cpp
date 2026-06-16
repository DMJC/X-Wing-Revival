/*
 *  NavPoint.cpp
 */

#include "NavPoint.h"
#include "XWingDefs.h"
#include "XWingGame.h"


NavPoint::NavPoint( uint32_t id ) : GameObject( id, XWing::Object::NAV_POINT )
{
	SystemNumber = 0;
	TargetSystem = 0;
	Visible      = true;
	Shape        = NULL;
}


NavPoint::~NavPoint()
{
}


bool NavPoint::PlayerShouldUpdateServer( void ) const { return false; }
bool NavPoint::ServerShouldUpdatePlayer( void ) const { return false; }
bool NavPoint::ServerShouldUpdateOthers( void ) const { return false; }
bool NavPoint::CanCollideWithOwnType( void ) const    { return false; }
bool NavPoint::CanCollideWithOtherTypes( void ) const { return false; }
bool NavPoint::IsMoving( void ) const                 { return false; }


void NavPoint::AddToInitPacket( Packet *packet, int8_t precision )
{
	packet->AddDouble( X );
	packet->AddDouble( Y );
	packet->AddDouble( Z );
	packet->AddString( Name );
	packet->AddUChar( SystemNumber );
	packet->AddUChar( TargetSystem );
	packet->AddUChar( Visible ? 1 : 0 );
	packet->AddString( VariableName );
}


void NavPoint::ReadFromInitPacket( Packet *packet, int8_t precision )
{
	X            = packet->NextDouble();
	Y            = packet->NextDouble();
	Z            = packet->NextDouble();
	Name         = packet->NextString();
	SystemNumber = packet->NextUChar();
	TargetSystem = packet->NextUChar();
	Visible      = packet->NextUChar() != 0;
	VariableName = packet->NextString();
}


void NavPoint::ClientInit( void )
{
	Shape = Raptor::Game->Res.GetModel("nav.obj");
	if( Shape && !Shape->Objects.size() )
		Shape = NULL;
}


void NavPoint::Draw( void )
{
	if( !Shape )
		return;

	XWingGame *game = (XWingGame*) Raptor::Game;

	bool visible = VariableName.empty() ? Visible : game->Data.PropertyAsBool( VariableName );
	if( !visible )
		return;

	Shape->DrawAt( this );
}
