#include "soundPlayer.hpp"

#include "SoundData/soundData.hpp"
#include <cmath>

#include "Utilities/debug.hpp"

using PopHead::Audio::SoundPlayer;

SoundPlayer::SoundPlayer()
	:mVolume(20.f)
{
	loadEverySound();
}

void SoundPlayer::loadEverySound()
{
	mSoundBuffers.load("resources/sounds/barretaShot.wav");
	mSoundBuffers.load("resources/sounds/zombieGetsAttacked.wav");
}

void SoundPlayer::playAmbientSound(const std::string& filePath)
{
	removeStoppedSounds();

	SoundData soundData = mSoundDataHolder.getSoundData(filePath);
	sf::Sound sound;
	sound.setBuffer(mSoundBuffers.get(filePath));
	sound.setVolume(mVolume * soundData.mVolumeMultiplier);
	sound.setLoop(soundData.mLoop);
	mSounds.emplace_back(std::move(sound));
	mSounds.back().play();
}

void SoundPlayer::playSpatialSound(const std::string& filePath, sf::Vector2f soundPosition)
{
	removeStoppedSounds();

	SoundData soundData = mSoundDataHolder.getSoundData(filePath);

	float volumeMultiplier = soundData.mVolumeMultiplier;
	float distance = hypotf(abs(mListenerPosition.x - soundPosition.x), abs(mListenerPosition.y - soundPosition.y));

	if(distance > soundData.mMin && distance < soundData.mMax) {
		float scope = soundData.mMax - soundData.mMin;
		volumeMultiplier *= ((scope - (distance - soundData.mMin)) / scope);
	}
	else if(distance > soundData.mMax)
		volumeMultiplier = 0.f;

	sf::Sound sound;
	sound.setBuffer(mSoundBuffers.get(filePath));
	sound.setVolume(mVolume * volumeMultiplier);
	sound.setLoop(soundData.mLoop);
	mSounds.emplace_back(std::move(sound));
	mSounds.back().play();
}

void SoundPlayer::removeStoppedSounds()
{
	mSounds.remove_if([](sf::Sound sound) {
		return sound.getStatus() == sf::Sound::Status::Stopped;
	});
}

void SoundPlayer::setVolume(float volume)
{
	mVolume = volume;
	for(auto& sound : mSounds) {
		sound.setVolume(volume);
	}
}

void SoundPlayer::removeEverySound()
{
	mSounds.clear();
}