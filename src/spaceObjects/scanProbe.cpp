#include <SFML/OpenGL.hpp>
#include "scanProbe.h"
#include "explosionEffect.h"
#include "main.h"
#include "playerSpaceship.h"

#include "scriptInterface.h"
REGISTER_SCRIPT_SUBCLASS_NO_CREATE(ScanProbe, SpaceObject)
{
}

REGISTER_MULTIPLAYER_CLASS(ScanProbe, "ScanProbe");
ScanProbe::ScanProbe()
: SpaceObject(100, "ScanProbe")
{
    lifetime = 60 * 60;
    hull = 2;

    registerMemberReplication(&owner_id);
    registerMemberReplication(&target_position);
    registerMemberReplication(&lifetime, 60.0);
    setRadarSignatureInfo(0.0, 0.2, 0);

    switch(irandom(1, 3))
    {
    case 1:
        model_info.setData("SensorBuoyMKI");
        break;
    case 2:
        model_info.setData("SensorBuoyMKII");
        break;
    default:
        model_info.setData("SensorBuoyMKIII");
        break;
    }

    setCallSign(string(getMultiplayerId()) + "P");
}

void ScanProbe::update(float delta)
{
    lifetime -= delta;
    if (lifetime <= 0.0)
        destroy();
    if ((target_position - getPosition()) > getRadius())
    {
        sf::Vector2f v = normalize(target_position - getPosition());
        setPosition(getPosition() + v * delta * probe_speed);
    }
}

void ScanProbe::collide(Collisionable* target, float force)
{
    if (!game_server)
        return;
    P<PlayerSpaceship> player = P<Collisionable>(target);
    if (!player)
        return;
    if (player -> getMultiplayerId() != owner_id)
        return;
    if ((getTarget() - getPosition()) > getRadius())
        return;
    player -> scan_probe_stock = player -> scan_probe_stock + 1;
    destroy();
}

void ScanProbe::drawOnRadar(sf::RenderTarget& window, sf::Vector2f position, float scale, bool long_range)
{
    sf::Sprite object_sprite;
    textureManager.setTexture(object_sprite, "ProbeBlip.png");
    object_sprite.setPosition(position);
    object_sprite.setColor(sf::Color(96, 192, 128));
    float size = 0.3;
    object_sprite.setScale(size, size);
    window.draw(object_sprite);
}

void ScanProbe::setOwner(P<SpaceObject> owner)
{
    if (!owner) return;

    setFactionId(owner->getFactionId());
    owner_id = owner->getMultiplayerId();
}
