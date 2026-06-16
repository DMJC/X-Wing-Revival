/*
 *  NavPoint.h
 */

#pragma once
class NavPoint;

#include "PlatformSpecific.h"
#include "GameObject.h"
#include "Model.h"
#include <string>


class NavPoint : public GameObject
{
public:
	std::string Name;
	uint8_t SystemNumber;
	uint8_t TargetSystem;
	bool Visible;
	std::string VariableName;
	Model *Shape;

	NavPoint( uint32_t id = 0 );
	virtual ~NavPoint();

	bool PlayerShouldUpdateServer( void ) const;
	bool ServerShouldUpdatePlayer( void ) const;
	bool ServerShouldUpdateOthers( void ) const;
	bool CanCollideWithOwnType( void ) const;
	bool CanCollideWithOtherTypes( void ) const;
	bool IsMoving( void ) const;

	void AddToInitPacket( Packet *packet, int8_t precision = 0 );
	void ReadFromInitPacket( Packet *packet, int8_t precision = 0 );

	void ClientInit( void );
	void Draw( void );
};
