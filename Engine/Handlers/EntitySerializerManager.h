#pragma once
#ifndef LEVIATHAN_ENTITYSERIALIZERMANAGER
#define LEVIATHAN_ENTITYSERIALIZERMANAGER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "SFML/Network/Packet.hpp"
#include "Entities/Bases/BaseObject.h"
#include "boost/function.hpp"
#include "Common/ThreadSafe.h"

namespace Leviathan{

	class EntitySerializerManager : public ThreadSafe{
	public:
		DLLEXPORT EntitySerializerManager();
        DLLEXPORT ~EntitySerializerManager();

        //! \brief Adds a new serializer
        //! \param serializer Pointer to the new serializer object. It will be deleted by this
        DLLEXPORT void AddSerializer(BaseEntitySerializer* serializer);
        
        //! \brief Deletes all stored serializers
        DLLEXPORT void Release();


        //! \brief Serializes an entity into a packet and links the entity to the connection
        //! \post The entity will automatically send following updates to the connection
        //! \return The packet which contains the data for the entity
        //! \note The object should be locked by the caller
        DLLEXPORT unique_ptr<sf::Packet> CreateInitialEntityMessageFor(BaseObject* object, ConnectionInfo* forwho);

        //! \brief Creates an entity from a packet
        //! \return True when the entity creation was attempted
        //! \param returnobj The inner pointer will be set to point towards the new entity or
        //! it will be NULL if the packet was corrupted or otherwise unusable. The object needs to
        //! be deleted by the caller
        DLLEXPORT bool CreateEntityFromInitialMessage(BaseObject** returnobj, sf::Packet &packet);

        //! \brief Applies an update from a packet
        //! \return True when a suitable deserializer was found and the target object was valid
        //! \param objectget Function pointer to from which the target can be acquired
        //! \note The object will be locked by this
        DLLEXPORT bool ApplyUpdateMessage(sf::Packet &packet, boost::function<BaseObject*(int)> objectget);


        DLLEXPORT static EntitySerializerManager* Get();

	protected:

        //! The vector of stored serializers
        //! Add your own serializers for custom types here
        std::vector<BaseEntitySerializer*> Serializers;
        
        static EntitySerializerManager* Staticinstance;
	};

}
#endif