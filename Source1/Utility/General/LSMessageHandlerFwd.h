#pragma once

#include "Prerequisites/LSTypes.h"
#include "String/LSString.h"

namespace ls
{
	/** @addtogroup General
	 *  @{
	 */

	/**
	 * Identifier for message used with the global messaging system.
	 *
	 * @note	
	 * Primary purpose of this class is to avoid expensive string compare, and instead use a unique message identifier for
	 * compare. Generally you want to create one of these using the message name, and then store it for later use.
	 * @note
	 * This class is not thread safe and should only be used on the sim thread.
	 */
	class LS_UTILITY_EXPORT MessageId
	{
	public:
		MessageId() = default;
		MessageId(const String& name);

		bool operator== (const MessageId& rhs) const
		{
			return (mMsgIdentifier == rhs.mMsgIdentifier);
		}
	private:
		friend class MessageHandler;

		static Map<String, UINT32> UniqueMessageIds;
		static UINT32 NextMessageId;

		UINT32 mMsgIdentifier = 0;
	};

	/** Handle to a subscription for a specific message in the global messaging system. */
	class LS_UTILITY_EXPORT HMessage
	{
	public:
		HMessage() = default;

		/** Disconnects the message listener so it will no longer receive events from the messaging system. */
		void disconnect();

	private:
		friend class MessageHandler;

		HMessage(UINT32 id);

		UINT32 mId = 0;
	};

	/**
	 * Sends a message using the global messaging system.
	 *
	 * @note	Sim thread only.
	 */
	void LS_UTILITY_EXPORT sendMessage(MessageId message);

	class MessageHandler;

	/** @} */
}
