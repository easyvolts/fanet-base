/*
 * fservice.cpp
 *
 *  Created on: Oct 31, 2018
 *      Author: sid
 */

#include <cmath>

#include "constrain.h"

#include "../fanet.h"
#include "fservice.h"

void FanetFrameService::setTemperature(float temp)
{
	temperature = temp;

	header |= 1<<FANETFRAMESERVICE_TEMP;
}

void FanetFrameService::setWind(float dir, float speed, float gust)
{
	windDir_deg = dir;
	windSpeed_kmph = speed;
	windGust_kmph = gust;

	header |= 1<<FANETFRAMESERVICE_WIND;
}

void FanetFrameService::setHumidity(float relHum)
{
	relHumidity = relHum;

	header |= 1<<FANETFRAMESERVICE_HUMIDITY;
}

void FanetFrameService::setPressure(float hPa)
{
	preasure_hPa = hPa;

	header |= 1<<FANETFRAMESERVICE_PRESSURE;
}


/* handle payload */
int16_t FanetFrameService::serialize(uint8_t*& buffer)
{
	/* prepare storage */
	payloadLength = 1 + 6;
	if(header & (1<<FANETFRAMESERVICE_TEMP))
		payloadLength++;
	if(header & (1<<FANETFRAMESERVICE_WIND))
		payloadLength+=3;
	if(header & (1<<FANETFRAMESERVICE_HUMIDITY))
		payloadLength++;
	if(header & (1<<FANETFRAMESERVICE_PRESSURE))
		payloadLength+=2;
	if(payload != nullptr)
		delete [] payload;
	payload = new uint8_t[payloadLength];
	if(payload == nullptr)
		return -1;

	/* mandatory subheader, position */
	header &= 0x7F;
	header |= (!!hasInet)<<FANETFRAMESERVICE_INET;
	payload[0] = header;
	coord2payload_absolut(fanet.position, &payload[1]);

	/* optional stuff */
	uint16_t idx = 1 + 6;
	if(header & (1<<FANETFRAMESERVICE_TEMP))
	{
		payload[idx++] = constrain((int)roundf(temperature*2.0f), -127, 127);
	}
	if(header & (1<<FANETFRAMESERVICE_WIND))
	{
		/* direction */
		float dirf = windDir_deg + fanet.heading;
		while(dirf < 0.0f)
			dirf += 360.0f;
		while(dirf >= 360.0f)
			dirf -= 360.0f;
		int dir = constrain((int)roundf(dirf*0.7083333334f), 0, 255);
		payload[idx++] = dir;

		/* speed */
		int speed = constrain((int)roundf(windSpeed_kmph*5.0f), 0, 635);
		if(speed > 127)
			payload[idx++] = ((speed+2)/5) | (1<<7);
		else
			payload[idx++] = speed & 0x7F;

		/* gust */
		int gust = constrain((int)roundf(windGust_kmph*5.0f), 0, 635);
		if(gust > 127)
			payload[idx++] = ((gust+2)/5) | (1<<7);
		else
			payload[idx++] = gust & 0x7F;
	}
	if(header & (1<<FANETFRAMESERVICE_HUMIDITY))
	{
		payload[idx++] = constrain((unsigned int)roundf(relHumidity*2.5f), 0U, 255U);
	}
	if(header & (1<<FANETFRAMESERVICE_PRESSURE))
	{
		uint16_t p = constrain((unsigned int)roundf((preasure_hPa-430.0f)*10.0f), 0U, 65535U);
		*((uint16_t*) &payload[idx]) = p;
		idx += 2;
	}

	return FanetFrame::serialize(buffer);
}

void FanetFrameService::decode(const uint8_t *payload, const uint16_t len, FanetNeighbor *neighbor)
{
	//not implemented/interested for now
}


