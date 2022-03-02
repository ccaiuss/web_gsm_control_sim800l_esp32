
class ChatHandler : public WebsocketHandler
{
public:
	static WebsocketHandler *create();
	void onMessage(WebsocketInputStreambuf *input);
	void onClose();
};

ChatHandler *activeClients[MAX_CLIENTS];

WebsocketHandler *ChatHandler::create()
{

	Serial.println("[WS] --> Client nou!");
	ChatHandler *handler = new ChatHandler();
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (activeClients[i] == nullptr)
		{
			activeClients[i] = handler;
			break;
		}
	}
	wsSendConfOneTime = true;
	return handler;
}

void ChatHandler::onClose()
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (activeClients[i] == this)
		{
			activeClients[i] = nullptr;
		}
	}
}

void ChatHandler::onMessage(WebsocketInputStreambuf *inbuf)
{
	std::ostringstream ss;
	std::string msg;
	ss << inbuf;
	msg = ss.str();
	
	DynamicJsonBuffer jsonBuffer;
	JsonObject &jsonMem = jsonBuffer.parseObject(msg.c_str());
	if (jsonMem.success())
	{
		if (jsonMem["cmd"].as<String>() == "ping")
		{
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (activeClients[i] != nullptr)
				{
					activeClients[i]->send("ping", SEND_TYPE_TEXT);
				}
			}
		}

		if (jsonMem["cmd"].as<String>() == "client")
		{
			Serial.println(msg.c_str());
			conf.modAntena = jsonMem["modAntena"].as<String>();
			conf.defaultConf = jsonMem["defaultConf"].as<String>();
			conf.zonaApel = jsonMem["zonaApel"];
			conf.passSms = jsonMem["passSms"].as<String>();

			for (int i = 1; i <= NRPINI; i++)
			{
				pinOut[i].sw = jsonMem["sw" + String(i)];
				pinOut[i].type = jsonMem["type" + String(i)];
				pinOut[i].delayOn = jsonMem["delayOn" + String(i)];
			}
			salvare();
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (activeClients[i] != nullptr)
				{
					activeClients[i]->send(msg.c_str(), SEND_TYPE_TEXT);
				}
			}
		}

		if (jsonMem["cmd"] == "restart")
		{
			ESP.restart();
		}
	}
}
