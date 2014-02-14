#ifndef LEVIATHAN_NETWORKRESPONSE
#define LEVIATHAN_NETWORKRESPONSE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "SFML/Network/Packet.hpp"
#include "NetworkHandler.h"


namespace Leviathan{

	

	//! Defines in what way a request was invalid also
	//! defines why a server disallowed a request
	enum NETWORKRESPONSE_INVALIDREASON{

		//! Returned when the connection is anonymous (the other client hasn't requested verified connection)
		NETWORKRESPONSE_INVALIDREASON_UNAUTHENTICATED,
		//! Returned when we don't implement the wanted action (for example if we are asked our server status and we aren't a server)
		NETWORKRESPONSE_INVALIDREASON_UNSUPPORTED,
		//! Server has maximum number of players
		NETWORKRESPONSE_INVALIDREASON_SERVERFULL,
		//! Server is not accepting players
		NETWORKRESPONSE_INVALIDREASON_SERVERNOTACCEPTINGPLAYERS,

		//! The server has used a custom rule to disallow this
		NETWORKRESPONSE_INVALIDREASON_SERVERCUSTOM
	};

	//! Defines server join protection status (who can join the server)
	enum NETWORKRESPONSE_SERVERJOINRESTRICT{

		//! Everyone can join the server 
		NETWORKRESPONSE_SERVERJOINRESTRICT_NONE,
		NETWORKRESPONSE_SERVERJOINRESTRICT_WHITELIST,
		NETWORKRESPONSE_SERVERJOINRESTRICT_INVITE,
		NETWORKRESPONSE_SERVERJOINRESTRICT_FRIENDS,
		NETWORKRESPONSE_SERVERJOINRESTRICT_PERMISSIONS,
		NETWORKRESPONSE_SERVERJOINRESTRICT_CUSTOM
	};

	//! Allows servers to tell clients what they are doing
	enum NETWORKRESPONSE_SERVERSTATUS{

		NETWORKRESPONSE_SERVERSTATUS_STARTING,
		NETWORKRESPONSE_SERVERSTATUS_RUNNING,
		NETWORKRESPONSE_SERVERSTATUS_SHUTDOWN,
		NETWORKRESPONSE_SERVERSTATUS_RESTART
	};


	enum NETWORKRESPONSETYPE{
		//! Sent in response to a NETWORKREQUESTTYPE_IDENTIFICATION contains a user readable string, game name, game version and leviathan version strings
		NETWORKRESPONSETYPE_IDENTIFICATIONSTRINGS,
		NETWORKRESPONSETYPE_KEEPALIVE,
		NETWORKRESPONSETYPE_CLOSECONNECTION,
		NETWORKRESPONSETYPE_REMOTECONSOLECLOSED,
		NETWORKRESPONSETYPE_REMOTECONSOLEOPENED,
		NETWORKRESPONSETYPE_INVALIDREQUEST,
		//! \brief Sent by a server when it disallows a made request
		NETWORKRESPONSETYPE_SERVERDISALLOW,
		//! \brief Sent by a server when a request is allowed
		NETWORKRESPONSETYPE_SERVERALLOW,
		//! Returns anonymous data about the server
		NETWORKRESPONSETYPE_SERVERSTATUS,
		NETWORKRESPONSETYPE_NONE
	};

	//! Base class for all data objects that can be sent with the NETWORKRESPONSETYPE
	//! \note Even though it cannot be required by the base class, sub classes should implement a constructor taking in a sf::Packet object
	class BaseNetworkResponseData{
	public:

		DLLEXPORT virtual ~BaseNetworkResponseData(){};

		DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet) = 0;
	};

	//! Stores data for NETWORKRESPONSETYPE_IDENTIFICATIONSTRINGS
	class NetworkResponseDataForIdentificationString : public BaseNetworkResponseData{
	public:
		DLLEXPORT NetworkResponseDataForIdentificationString(sf::Packet &frompacket);
		DLLEXPORT NetworkResponseDataForIdentificationString(const wstring &userreadableidentification, const wstring &gamename, const wstring &gameversion, 
			const wstring &leviathanversion);

		DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);
		

		// Data //
		wstring UserReadableData;
		wstring GameName;
		wstring GameVersionString;
		wstring LeviathanVersionString;
	};

	//! Stores data for NETWORKRESPONSETYPE_INVALIDREQUEST
	class NetworkResponseDataForInvalidRequest : public BaseNetworkResponseData{
	public:
		DLLEXPORT NetworkResponseDataForInvalidRequest(sf::Packet &frompacket);
		DLLEXPORT NetworkResponseDataForInvalidRequest(NETWORKRESPONSE_INVALIDREASON reason, const wstring &additional = wstring());
		DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);


		NETWORKRESPONSE_INVALIDREASON Invalidness;
		wstring AdditionalInfo;
	};

	//! Stores data for NETWORKRESPONSETYPE_SERVERSTATUS
	class NetworkResponseDataForServerStatus : public BaseNetworkResponseData{
	public:
		DLLEXPORT NetworkResponseDataForServerStatus(sf::Packet &frompacket);
		DLLEXPORT NetworkResponseDataForServerStatus(const wstring &servername, bool isjoinable, NETWORKRESPONSE_SERVERJOINRESTRICT whocanjoin,
			int players, int maxplayers, int bots, NETWORKRESPONSE_SERVERSTATUS currentstatus, int serverflags);
		DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);

		//! Contains the name of the server, should be limited to max 100 letters
		wstring ServerNameString;
		//! States if the server is joinable (has started, doesn't take slots into account)
		bool Joinable;

		//! Defines the type of join authentication the server uses (restricts who can join)
		NETWORKRESPONSE_SERVERJOINRESTRICT JoinRestriction;

		//! Current human players on the server
		int Players;
		//! Maximum human players
		int MaxPlayers;

		//! Current bots on the server
		int Bots;

		//! The current status of the server. Used to define what the server is doing
		NETWORKRESPONSE_SERVERSTATUS ServerStatus;

		//! The flags of the server. These can be used based on the game for example to define game mode or level requirements or something else
		int AdditionalFlags;
	};

	//! \brief Stores data about a server disallow response
	class NetworkResponseDataForServerDisallow : public BaseNetworkResponseData{
	public:
		DLLEXPORT NetworkResponseDataForServerDisallow(sf::Packet &frompacket);
		DLLEXPORT NetworkResponseDataForServerDisallow(NETWORKRESPONSE_INVALIDREASON reason, const wstring &message = L"Default disallow");
		DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);

		//! \brief An user readable disallow string
		//! \note Should be limited to a maximum of 100 characters
		wstring Message;

		//! The reason why this request was dropped
		NETWORKRESPONSE_INVALIDREASON Reason;
	};

	//! \brief Stores data about a server allow response
	class NetworkResponseDataForServerAllow : public BaseNetworkResponseData{
	public:
		DLLEXPORT NetworkResponseDataForServerAllow(sf::Packet &frompacket);
		DLLEXPORT NetworkResponseDataForServerAllow();
		DLLEXPORT virtual void AddDataToPacket(sf::Packet &packet);


	};


	class NetworkResponse : public Object{
	public:
		DLLEXPORT NetworkResponse(int inresponseto, PACKET_TIMEOUT_STYLE timeout, int timeoutvalue);
		// This is for constructing these on the receiver side //
		DLLEXPORT NetworkResponse(sf::Packet &receivedresponse);
		DLLEXPORT ~NetworkResponse();

		// Named "constructors" for different types //
		DLLEXPORT void GenerateIdentificationStringResponse(NetworkResponseDataForIdentificationString* newddata);
		DLLEXPORT void GenerateInvalidRequestResponse(NetworkResponseDataForInvalidRequest* newddata);
		DLLEXPORT void GenerateServerStatusResponse(NetworkResponseDataForServerStatus* newddata);
		DLLEXPORT void GenerateServerDisallowResponse(NetworkResponseDataForServerDisallow* newddata);
		DLLEXPORT void GenerateServerAllowResponse(NetworkResponseDataForServerAllow* newddata);

		DLLEXPORT void GenerateKeepAliveResponse();
		DLLEXPORT void GenerateCloseConnectionResponse();
		DLLEXPORT void GenerateRemoteConsoleOpenedResponse();
		DLLEXPORT void GenerateRemoteConsoleClosedResponse();


		DLLEXPORT void GenerateEmptyResponse();

		DLLEXPORT NETWORKRESPONSETYPE GetTypeOfResponse();

		DLLEXPORT sf::Packet GeneratePacketForResponse();

		DLLEXPORT NetworkResponseDataForIdentificationString* GetResponseDataForIdentificationString();
		DLLEXPORT NetworkResponseDataForServerStatus* GetResponseDataForServerStatus();

		DLLEXPORT int GetTimeOutValue();
		DLLEXPORT PACKET_TIMEOUT_STYLE GetTimeOutType();

		// De-coding functions //

		DLLEXPORT int GetResponseID();

	protected:

		int ResponseID;


		int TimeOutValue;
		PACKET_TIMEOUT_STYLE TimeOutStyle;

		NETWORKRESPONSETYPE ResponseType;

		// Holds the pointer to the struct that holds the response data //
		BaseNetworkResponseData* ResponseData;
	};

}
#endif