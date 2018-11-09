# Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`define `[`PEDAL_POWER_MODIFIER`](#transport_8h_1afbbb18ca59257b9b19eced9dcb7c7973)            | The modifier value if pedal power is used.
`enum `[`TransportType`](#transport_8h_1aca1e72535e7f260e54ed8bbf984dade9)            | Enum class for transport types.
`namespace `[`transport`](#namespacetransport) | 

## Members

#### `define `[`PEDAL_POWER_MODIFIER`](#transport_8h_1afbbb18ca59257b9b19eced9dcb7c7973) {#transport_8h_1afbbb18ca59257b9b19eced9dcb7c7973}

The modifier value if pedal power is used.

This definition exists in the default namespace and is ungrouped. It will *not* be displayed if the `groups` options is used.

#### `enum `[`TransportType`](#transport_8h_1aca1e72535e7f260e54ed8bbf984dade9) {#transport_8h_1aca1e72535e7f260e54ed8bbf984dade9}

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
Bycicle            | Bycicle type
RacingBike            | Racing bike type
RacingBike            | Mountain bike type

Enum class for transport types.

This definition exists in the default namespace and is ungrouped. It will *not* be displayed if the `groups` options is used.

# namespace `transport` {#namespacetransport}

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`class `[`transport::Bicycle`](#classtransport_1_1Bicycle) | Standard bicycle class.
`class `[`transport::MountainBike`](#classtransport_1_1MountainBike) | Mountain bike implementation of a `[Bicycle](#classtransport_1_1Bicycle)`.
`class `[`transport::RacingBike`](#classtransport_1_1RacingBike) | Racing bike class.

# class `transport::Bicycle` {#classtransport_1_1Bicycle}

Standard bicycle class.

[Bicycle](#classtransport_1_1Bicycle) implements a standard bicycle. Bicycles are a useful way of transporting oneself, without too much effort (unless you go uphill or against the wind). If there are a lot of people on the road, you can use `RingBell` to ring your bell (**note**, not all bicycles have bells!).

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public virtual void `[`PedalHarder`](#classtransport_1_1Bicycle_1a7df6cce8f18012fb07bef5be9dadd8ef)`()` | PedalHarder makes you go faster (usually).
`public virtual void `[`RingBell`](#classtransport_1_1Bicycle_1a7d2be572f09c78b4d4ae38ef22f3e98b)`()` | Ring bell on the bike.
`public virtual  `[`~Bicycle`](#classtransport_1_1Bicycle_1a5f62d09b772a7705634bfb3551803c25)`()` | Default destructor.

## Members

#### `public virtual void `[`PedalHarder`](#classtransport_1_1Bicycle_1a7df6cce8f18012fb07bef5be9dadd8ef)`()` {#classtransport_1_1Bicycle_1a7df6cce8f18012fb07bef5be9dadd8ef}

PedalHarder makes you go faster (usually).

#### `public virtual void `[`RingBell`](#classtransport_1_1Bicycle_1a7d2be572f09c78b4d4ae38ef22f3e98b)`()` {#classtransport_1_1Bicycle_1a7d2be572f09c78b4d4ae38ef22f3e98b}

Ring bell on the bike.

RingBell rings the bell on the bike. Note that not all bikes have bells.

#### `public virtual  `[`~Bicycle`](#classtransport_1_1Bicycle_1a5f62d09b772a7705634bfb3551803c25)`()` {#classtransport_1_1Bicycle_1a5f62d09b772a7705634bfb3551803c25}

Default destructor.

# class `transport::MountainBike` {#classtransport_1_1MountainBike}

```
class transport::MountainBike
  : public transport::Bicycle
```  

Mountain bike implementation of a `[Bicycle](#classtransport_1_1Bicycle)`.

[MountainBike](#classtransport_1_1MountainBike) is an implementation of a [Bicycle](#classtransport_1_1Bicycle) providing a bike for cycling on rough terrain. Mountain bikes are pretty cool because they have stuff like **Suspension** (and you can even adjust it using SetSuspension). If you're looking for a bike for use on the road, you might be better off using a [RacingBike](#classtransport_1_1RacingBike) though.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public bool `[`SetSuspension`](#classtransport_1_1MountainBike_1a04caecd7e5ff7572b6ac1dc283510301)`(double stiffness)` | Set suspension stiffness.  the suspension stiffness.
`public template<>`  <br/>`inline bool `[`ChangeBreak`](#classtransport_1_1MountainBike_1afd02513876a196e98acaacdc555aeb52)`(BreakType breakType)` | Change the break type.  the break type.  the type of the break.

## Members

#### `public bool `[`SetSuspension`](#classtransport_1_1MountainBike_1a04caecd7e5ff7572b6ac1dc283510301)`(double stiffness)` {#classtransport_1_1MountainBike_1a04caecd7e5ff7572b6ac1dc283510301}

Set suspension stiffness.  the suspension stiffness.

SetSuspension changes the stiffness of the suspension on the bike. The method will return false if the stiffness could not be adjusted.

#### Returns
true if the suspension was adjusted successfully, false otherwise.

#### `public template<>`  <br/>`inline bool `[`ChangeBreak`](#classtransport_1_1MountainBike_1afd02513876a196e98acaacdc555aeb52)`(BreakType breakType)` {#classtransport_1_1MountainBike_1afd02513876a196e98acaacdc555aeb52}

Change the break type.  the break type.  the type of the break.

ChangesBreak changes the type of break fitted to the bike. The method will return false if the break type could not be fitted.

#### Returns
true if the break was adjusted successfully. false otherise

# class `transport::RacingBike` {#classtransport_1_1RacingBike}

```
class transport::RacingBike
  : public transport::Bicycle
```  

Racing bike class.

[RacingBike](#classtransport_1_1RacingBike) is a special kind of bike which can go much faster on the road, with much less effort (even uphill!). It doesn't make sense to call `RingBell` on a racing bike for they don't have bells.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public virtual void `[`PedalHarder`](#classtransport_1_1RacingBike_1ab557c5727daa07a5001782d5dcd46c5b)`()` | PedalHarder makes you go faster (usually).
`public virtual void `[`RingBell`](#classtransport_1_1RacingBike_1ad32dc3b06a453fba3e20329842bb318b)`()` | Ring bell on the bike.

## Members

#### `public virtual void `[`PedalHarder`](#classtransport_1_1RacingBike_1ab557c5727daa07a5001782d5dcd46c5b)`()` {#classtransport_1_1RacingBike_1ab557c5727daa07a5001782d5dcd46c5b}

PedalHarder makes you go faster (usually).

#### `public virtual void `[`RingBell`](#classtransport_1_1RacingBike_1ad32dc3b06a453fba3e20329842bb318b)`()` {#classtransport_1_1RacingBike_1ad32dc3b06a453fba3e20329842bb318b}

Ring bell on the bike.

RingBell rings the bell on the bike. Note that not all bikes have bells.

Generated by [Moxygen](https://sourcey.com/moxygen)