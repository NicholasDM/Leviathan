// ------------------- //
// This file is part of Leviathan engine. This file is based on Hieroglyph3 Rendering Engine file.
// Files are modified more or less. See below for Original license 
// ------------------- //

//--------------------------------------------------------------------------------
// This file is a portion of the Hieroglyph 3 Rendering Engine.  It is distributed
// under the MIT License, available in the root of this distribution and 
// at the following URL:
//
// http://www.opensource.org/licenses/mit-license.php
//
// Copyright (c) 2003-2010 Jason Zink 
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// IntrRay3fBox3f
//
//--------------------------------------------------------------------------------
#ifndef LEVIATHAN_INTERSECTORBOX3
#define LEVIATHAN_INTERSECTORBOX3
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Intersector.h"
#include "Ray3f.h"
#include "Box3f.h"

namespace Leviathan{

	class IntrRay3fBox3f : public Intersector
	{
	public:
		IntrRay3fBox3f( Ray3f& ray, Box3f& box );
		virtual ~IntrRay3fBox3f( );
	
		virtual bool Test();
		virtual bool Find();

		bool Clip ( float fDenom, float fNumer, float& rfT0, float& rfT1 );

	public:
		Ray3f			m_Ray;
		Box3f			m_Box;

		Vector3f		m_aPoints[2];
		float			m_afRayT[2];
		int				m_iQuantity;
	};
};
//--------------------------------------------------------------------------------
#endif // IntrRay3fBox3f_h