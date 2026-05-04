/*
 *  Obstacle.cpp
 */

#include "Obstacle.h"

#include <cmath>
#include <algorithm>
#include "XWingDefs.h"
#include "XWingGame.h"
#include "XWingServer.h"
#include "Math3D.h"
#include "Shot.h"
#include "Asteroid.h"


Obstacle::Obstacle( uint32_t id )
: BlastableObject( id, XWing::Object::OBSTACLE )
{
	Length = Height = Width = 0.;
	CollisionName = "the wall";
}


Obstacle::Obstacle( const Obstacle *other )
: BlastableObject( 0, XWing::Object::OBSTACLE )
{
	Copy( other );
	
	Length = Height = Width = 0.;
	ClientModel   = other->ClientModel;
	CollisionName = other->CollisionName;
	
	CopyModel( &(other->Shape) );
}


Obstacle::Obstacle( const std::string &client_model, const Model *server_model, const Pos3D &pos, const std::string &name )
: BlastableObject( 0, XWing::Object::OBSTACLE )
{
	Length = Height = Width = 0.;
	ClientModel   = client_model;
	CollisionName = name;
	
	Copy( &pos );
	
	if( server_model )
		CopyModel( server_model );
	else
		LoadModel( client_model );
}


Obstacle::Obstacle( const std::string &client_model, const std::string &server_model, const std::string &name )
: BlastableObject( 0, XWing::Object::OBSTACLE )
{
	Length = Height = Width = 0.;
	ClientModel   = client_model;
	CollisionName = name;
	
	LoadModel( server_model );
}


Obstacle::~Obstacle()
{
}


void Obstacle::CopyModel( const Model *model )
{
	if( model )
		Shape.BecomeCopy( model );
	else
		Shape.Clear();
}


void Obstacle::LoadModel( const std::string &model )
{
	Shape.LoadOBJ( std::string("Models/") + model, false );
	Shape.GetMaxRadius();
	
	// Retain info to send to clients.
	if( ClientModel.empty() )
		ClientModel = model;
}


void Obstacle::ScaleBy( float scale )
{
	ScaleBy( scale, scale, scale );
}


void Obstacle::ScaleBy( float scale_f, float scale_u, float scale_r )
{
	Shape.ScaleBy( scale_f, scale_u, scale_r );
}


void Obstacle::ScaleTo( float length, float height, float width )
{
	Shape.ScaleTo( length, height, width );
}


bool Obstacle::SnapTo( const Obstacle *other, double max_dist )
{
	std::map<std::string,ModelObject*>::const_iterator this_snap = Shape.Objects.find("snap");
	if( (this_snap == Shape.Objects.end()) || this_snap->second->Lines.empty() )
		return false;
	
	std::map<std::string,ModelObject*>::const_iterator other_snap = other->Shape.Objects.find("snap");
	if( (other_snap == other->Shape.Objects.end()) || other_snap->second->Lines.empty() )
		return false;
	
	Pos3D best_snap( this );
	double best_dist = FLT_MAX;
	bool snapped = false;
	
	for( std::vector< std::vector<Vec3D> >::const_iterator snap1 = this_snap->second->Lines.begin(); snap1 != this_snap->second->Lines.end(); snap1 ++ )
	{
		Vec3D this_pt = snap1->front();
		Pos3D this_snap( this );
		this_snap.MoveAlong( &(this->Fwd),   this_pt.X );
		this_snap.MoveAlong( &(this->Up),    this_pt.Y );
		this_snap.MoveAlong( &(this->Right), this_pt.Z );
		Vec3D this_snap_dir;
		if( snap1->size() >= 2 )
		{
			Vec3D inward = snap1->at(1);
			Pos3D in_pt( this );
			in_pt.MoveAlong( &(this->Fwd),   inward.X );
			in_pt.MoveAlong( &(this->Up),    inward.Y );
			in_pt.MoveAlong( &(this->Right), inward.Z );
			this_snap_dir = this_snap - in_pt;
		}
		
		for( std::vector< std::vector<Vec3D> >::const_iterator snap2 = other_snap->second->Lines.begin(); snap2 != other_snap->second->Lines.end(); snap2 ++ )
		{
			Vec3D other_pt = snap2->front();
			Pos3D other_snap( other );
			other_snap.MoveAlong( &(other->Fwd),   other_pt.X );
			other_snap.MoveAlong( &(other->Up),    other_pt.Y );
			other_snap.MoveAlong( &(other->Right), other_pt.Z );
			Vec3D other_snap_dir;
			if( snap2->size() >= 2 )
			{
				Vec3D inward = snap2->at(1);
				Pos3D in_pt( other );
				in_pt.MoveAlong( &(other->Fwd),   inward.X );
				in_pt.MoveAlong( &(other->Up),    inward.Y );
				in_pt.MoveAlong( &(other->Right), inward.Z );
				other_snap_dir = other_snap - in_pt;
			}
			
			// Avoid overlapping obstacles.  If the snap vectors point the same direction, do not snap these points.
			if( this_snap_dir.Dot(&other_snap_dir) > 0. )
				continue;
			
			Vec3D diff = other_snap - this_snap;
			double dist = diff.Length();
			
			if( max_dist && (dist < max_dist) )
				continue;
			
			if( dist < best_dist )
			{
				best_snap = *this + diff;  // FIXME: Rotate?
				best_dist = dist;
				snapped = true;
			}
		}
	}
	
	if( snapped )
		Copy( &best_snap );
	return snapped;
}


void Obstacle::ClientInit( void )
{
	if( ! ClientModel.empty() )
	{
		Model *model = Raptor::Game->Res.GetModel( ClientModel );
		if( model )
		{
			Shape.BecomeInstance( model );
			Shape.ScaleTo( Length, Height, Width );
		}
	}
}


bool Obstacle::PlayerShouldUpdateServer( void ) const
{
	return false;
}

bool Obstacle::ServerShouldUpdatePlayer( void ) const
{
	return false;
}

bool Obstacle::ServerShouldUpdateOthers( void ) const
{
	return (RollRate || PitchRate || YawRate || MotionVector.Length());
}

bool Obstacle::CanCollideWithOwnType( void ) const
{
	return false;
}

bool Obstacle::CanCollideWithOtherTypes( void ) const
{
	return true;
}

bool Obstacle::ComplexCollisionDetection( void ) const
{
	return true;
}


void Obstacle::AddToInitPacket( Packet *packet, int8_t precision )
{
	GameObject::AddToInitPacket( packet, precision );
	
	packet->AddString( ClientModel );
	packet->AddFloat( Shape.GetLength() );
	packet->AddFloat( Shape.GetHeight() );
	packet->AddFloat( Shape.GetWidth() );
}


void Obstacle::ReadFromInitPacket( Packet *packet, int8_t precision )
{
	GameObject::ReadFromInitPacket( packet, precision );
	
	ClientModel = packet->NextString();
	Length      = packet->NextFloat();
	Height      = packet->NextFloat();
	Width       = packet->NextFloat();
}


void Obstacle::AddToUpdatePacketFromServer( Packet *packet, int8_t precision )
{
	GameObject::AddToUpdatePacketFromServer( packet, precision );
}


void Obstacle::ReadFromUpdatePacketFromServer( Packet *packet, int8_t precision )
{
	GameObject::ReadFromUpdatePacketFromServer( packet, precision );
}


bool Obstacle::WillCollide( const GameObject *other, double dt, std::string *this_object, std::string *other_object, Pos3D *loc, double *when ) const
{
	double block_size = 0.;  // Default to automatic (use longest triangle edge as block size).
	
	uint32_t other_type = other->Type();
	
	if( other_type == XWing::Object::SHOT )
	{
		if( Math3D::MinimumDistance( this, &MotionVector, other, &(other->MotionVector), dt ) > Shape.MaxRadius )
			return false;
		
		Vec3D relative_motion = (other->MotionVector - MotionVector) * dt;
		Pos3D end( other );
		end += relative_motion;
		
		ModelArrays array_inst;
		Pos3D this_loc;
		for( std::map<std::string,ModelObject*>::const_iterator obj_iter = Shape.Objects.begin(); obj_iter != Shape.Objects.end(); obj_iter ++ )
		{
			// Get the worldspace center of object.
			Pos3D modelspace_center = obj_iter->second->CenterPoint;
			Vec3D offset = Fwd * modelspace_center.X + Up * modelspace_center.Y + Right * modelspace_center.Z;
			Pos3D center = *this + offset;
			
			// If these two objects don't pass near each other, don't bother checking faces.
			if( Math3D::MinimumDistance( &center, &MotionVector, other, &(other->MotionVector), dt ) > obj_iter->second->MaxRadius )
				continue;
			
			for( std::map<std::string,ModelArrays*>::const_iterator array_iter = obj_iter->second->Arrays.begin(); array_iter != obj_iter->second->Arrays.end(); array_iter ++ )
			{
				array_inst.BecomeInstance( array_iter->second );
				array_inst.MakeWorldSpace( this );
				
				for( size_t i = 0; i + 2 < array_inst.VertexCount; i += 3 )
				{
					double dist = Math3D::LineSegDistFromFace( other, &end, array_inst.WorldSpaceVertexArray + i*3, 3, &this_loc );
					if( dist < 0.1 )
					{
						if( loc )
							loc->Copy( &this_loc );
						if( this_object )
							*this_object = obj_iter->first;
						return true;
					}
				}
			}
		}
		
		return false;
	}
	
	else if( other_type == XWing::Object::SHIP )
	{
		Ship *ship = (Ship*) other;
		
		// Obstacles can't be destroyed, so we don't care if a dead ship is colliding with it.
		if( ship->Health <= 0. )
			return false;
		
		if( ship->ComplexCollisionDetection() )
		{
			// If they're nowhere near each other, don't bother getting fancy.
			if( Math3D::MinimumDistance( this, &MotionVector, ship, &(ship->MotionVector), dt ) > (Shape.MaxRadius + ship->Shape.MaxRadius) )
				return false;
			
			// Don't detect collisions with destroyed subsystems.
			std::set<std::string> objects2;
			for( std::map<std::string,ModelObject*>::const_iterator other_obj_iter = ship->Shape.Objects.begin(); other_obj_iter != ship->Shape.Objects.end(); other_obj_iter ++ )
			{
				std::map<std::string,double>::const_iterator subsystem_iter = ship->Subsystems.find( other_obj_iter->first );
				if( (subsystem_iter == ship->Subsystems.end()) || (subsystem_iter->second > 0.) )
					objects2.insert( other_obj_iter->first );
			}
			
			Vec3D relative_motion = (ship->MotionVector - MotionVector) * dt;
			Vec3D *motion2 = &relative_motion;
			if( ship->Category() == ShipClass::CATEGORY_CAPITAL )
			{
				// Performance tweaks.
				motion2 = NULL;
				block_size = 64.;
			}
			
			return Shape.CollidesWithModel( this, loc, NULL, this_object, 0., 0, &(ship->Shape), ship, motion2, &objects2, other_object, ship->Exploded(), ship->ExplosionSeed(), block_size );
		}
		
		// The ship uses a simple spherical collision model.
		return WillCollideWithSphere( ship, ship->Radius(), dt, this_object, loc, when );
	}
	
	else if( other_type == XWing::Object::ASTEROID )
	{
		Asteroid *asteroid = (Asteroid*) other;
		return WillCollideWithSphere( asteroid, asteroid->Radius, dt, this_object, loc, when );
	}
	
	return false;
}


bool Obstacle::WillCollideWithSphere( const GameObject *other, double other_radius, double dt, std::string *this_object, Pos3D *loc, double *when ) const
{
	// FIXME: Set *when = ?
	
	if( Math3D::MinimumDistance( this, &MotionVector, other, &(other->MotionVector), dt ) > (Shape.MaxRadius + other_radius) )
		return false;
	
	Vec3D relative_motion = (other->MotionVector - MotionVector) * dt;
	
	return Shape.CollidesWithSphere( this, loc, NULL, this_object, 0., 0, other, &relative_motion, other_radius );
}


void Obstacle::Update( double dt )
{
	GameObject::Update( dt );
}


void Obstacle::Draw( void )
{
	// Cull model back faces to reduce z-fighting and improve performance.
	glEnable( GL_CULL_FACE );
	
	Shape.Draw( this );
	
	glDisable( GL_CULL_FACE );
}


Shader *Obstacle::WantShader( void ) const
{
	/*
	// Use ship model shader if obstacle has any blastpoints.
	if( BlastPoints.size() )
		return NULL;
	return Raptor::Game->Res.GetShader("asteroid");
	*/
	
	// Always use full model shader so obstacles can have GlowMaps.
	return NULL;
}
