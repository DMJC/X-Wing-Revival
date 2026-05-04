/*
 *  Obstacle.h
 */

#pragma once
class Obstacle;
class ObstacleEngine;

#include "PlatformSpecific.h"

#include "BlastableObject.h"
#include <string>
#include <map>
#include <cfloat>
#include "Model.h"


class Obstacle : public BlastableObject
{
public:
	std::string ClientModel;
	Model Shape;
	std::string CollisionName;
	
	Obstacle( uint32_t id = 0 );
	Obstacle( const Obstacle *other );
	Obstacle( const std::string &client_model, const Model *server_model, const Pos3D &pos, const std::string &name = "the wall" );
	Obstacle( const std::string &client_model, const std::string &server_model, const std::string &name = "the wall" );
	virtual ~Obstacle();
	
	void CopyModel( const Model *model );
	void LoadModel( const std::string &model );
	void ScaleBy( float scale );
	void ScaleBy( float scale_f, float scale_u, float scale_r );
	void ScaleTo( float scale_f, float scale_u, float scale_r );
	bool SnapTo( const Obstacle *other, double max_dist = 0. );
	
	void ClientInit( void );
	
	bool PlayerShouldUpdateServer( void ) const;
	bool ServerShouldUpdatePlayer( void ) const;
	bool ServerShouldUpdateOthers( void ) const;
	bool CanCollideWithOwnType( void ) const;
	bool CanCollideWithOtherTypes( void ) const;
	bool ComplexCollisionDetection( void ) const;
	
	void AddToInitPacket( Packet *packet, int8_t precision = 0 );
	void ReadFromInitPacket( Packet *packet, int8_t precision = 0 );
	void AddToUpdatePacketFromServer( Packet *packet, int8_t precision = 0 );
	void ReadFromUpdatePacketFromServer( Packet *packet, int8_t precision = 0 );
	
	bool WillCollide( const GameObject *other, double dt, std::string *this_object = NULL, std::string *other_object = NULL, Pos3D *loc = NULL, double *when = NULL ) const;
	bool WillCollideWithSphere( const GameObject *other, double other_radius, double dt, std::string *this_object, Pos3D *loc = NULL, double *when = NULL ) const;
	void Update( double dt );
	
	void Draw( void );
	Shader *WantShader( void ) const;
	
private:
	float Length, Height, Width;
};
