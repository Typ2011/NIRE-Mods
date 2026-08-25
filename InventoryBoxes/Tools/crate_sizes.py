"""Derive per-crate inventory limits from the crate's actual mesh size.

Reads the bounding box out of each crate mesh's .xob header, then rewrites
MaxCumulativeVolume and MaxItemSize in every PrefabsEditable/**/*.et crate so a
small ammo can no longer holds as much as a pallet of equipment boxes.

Run after adding a crate variant, or after retuning the constants below:

    python Tools/crate_sizes.py            # show the table, change nothing
    python Tools/crate_sizes.py --write    # apply to the prefabs

Calibration
-----------
UNITS_PER_LITRE is anchored to vanilla containers: the ALICE Medium backpack
holds about 30 real litres and is authored as MaxCumulativeVolume 10000, so one
litre is worth roughly 333 units. Reforger's volume scale is abstract - items
are inflated relative to real life - so matching vanilla containers matters more
than matching reality.

FILL is the fraction of the outer bounding box that is actually usable space:
walls, lid, and the gaps in a stack of crates are not storage.

UseCapacityCoefficient 0 is written alongside the volume and is not optional.
Without it the engine sizes the storage from its slot count instead of from
MaxCumulativeVolume, and a 100-slot crate then swallows anything regardless of
the volume set here. Vanilla writes `UseCapacityCoefficient 0` in 108 prefabs
and never writes 1, so the class default is the coefficient path; the crates
inherit from AmmoBox_Base, which does not turn it off.

BIG_CRATE_METRES splits the crates into the two item-size gates below. Vanilla
item classes: 5 grenades/flares, 8x8x20 mortar shells, 15 magazines and small
rockets, 20 carbines and PG-7 rockets, 25 rifles, 30 SVD/M21, 35 the largest
launchers. So 20 admits everything a small ammo can plausibly holds while still
refusing full-size rifles, and 40 admits everything in the game.
"""

import argparse
import pathlib
import re
import struct
import typing

ROOT = pathlib.Path(__file__).resolve().parent.parent
GAME_DATA = pathlib.Path(r"F:\Reforger extracted\data")

UNITS_PER_LITRE = 333
FILL = 0.6
BIG_CRATE_METRES = 0.9
SMALL_ITEM_SIZE = "20 20 20"
BIG_ITEM_SIZE = "40 40 40"

# Cargo hold of a transport truck (M923A1_transport, Ural4320_transport). Every
# crate must be loadable into one, which main() enforces.
TRUCK_CARGO_VOLUME = 1000000
TRUCK_MAX_ITEM_SIZE = [200, 200, 200]

# The crate's own transport cost runs on its own scale, anchored to that truck:
# its 1000000 stands for a cargo bed of roughly 8 cubic metres. That puts about
# two of the largest pallet stacks or thirty wooden crates on one truck, which is
# what fits in reality. Do not reuse UNITS_PER_LITRE here - that one is anchored
# to worn containers, and vehicles are authored on a much coarser scale.
ITEM_UNITS_PER_LITRE = 125

# Grid footprint of the crate itself, by outer volume in litres. ESlotSize tops
# out at SLOT_3x3, so the stacks would otherwise all share the largest tier.
SLOT_TIERS = [(15, "SLOT_1x1"), (60, "SLOT_2x1"), (250, "SLOT_2x2")]
LARGEST_SLOT = "SLOT_3x3"

# Footprints set by eye on the model, matched against the prefab name and
# winning over SLOT_TIERS. Volume alone sorts the pallet stacks badly: the V1 and
# V2 stacks are broad but low, so they read as 2x2 despite measuring the same
# litres as the V5 stack, which stays 3x3. Only the stacks carry a _Vn suffix,
# and each key also catches its _covered twin.
SLOT_OVERRIDES = {"_V1": "SLOT_2x2", "_V2": "SLOT_2x2", "_V3": "SLOT_3x3"}

# Tare weight of the empty crate, in kilograms. A crate is a hollow shell, so its
# mass tracks surface area rather than the volume it encloses - it goes with
# litres ** (2/3), not with litres. Anchored on the wooden weapon crate at 20 kg,
# what a real 48-inch ammo crate weighs empty; the same coefficient puts the .50
# cal can at 2 kg against a real M2A1's 2.7 kg.
#
# Known ceiling: the pallet stacks are piles of separate crates, not one hollow
# box, so treating them as a single shell undercounts them - the largest lands at
# 104 kg where summing its crates suggests roughly twice that. Nothing gates on
# crate weight (carry ignores it, and cargo volume binds long before any vehicle
# weight limit), so this is left as the simpler single law.
TARE_WEIGHT_COEFFICIENT = 0.47

MESH_RE = re.compile(r'm_Mesh "\{[0-9A-F]+\}([^"]+)"')


def mesh_bounds(xob):
    """Return (min_x, min_y, min_z, max_x, max_y, max_z) from an .xob header."""
    head = xob.read_bytes()[:0x30]
    if head[:4] != b"FORM" or head[8:12] != b"XOB9" or head[12:16] != b"HEAD":
        raise ValueError(f"not an XOB9 mesh: {xob}")
    return struct.unpack_from("<6f", head, 0x18)


def crate_extents(prefab):
    """Union the bounding boxes of every mesh the prefab previews."""
    meshes = MESH_RE.findall(prefab.read_text())
    if not meshes:
        return None

    lo = [float("inf")] * 3
    hi = [float("-inf")] * 3
    for mesh in meshes:
        bounds = mesh_bounds(GAME_DATA / mesh)
        for axis in range(3):
            lo[axis] = min(lo[axis], bounds[axis])
            hi[axis] = max(hi[axis], bounds[axis + 3])
    return [hi[axis] - lo[axis] for axis in range(3)]


def outer_litres(extents):
    return extents[0] * extents[1] * extents[2] * 1000


def limits(extents):
    """What the crate can hold."""
    volume = round(outer_litres(extents) * FILL * UNITS_PER_LITRE, -2)
    item_size = BIG_ITEM_SIZE if max(extents) >= BIG_CRATE_METRES else SMALL_ITEM_SIZE
    return int(volume), item_size


class Item(typing.NamedTuple):
    """The crate's own attributes, as it appears inside another inventory."""

    volume: int
    dimensions: str
    slot: str
    weight: int


def as_item(extents, name=""):
    """What the crate costs when it is itself loaded into something else.

    Volume runs on ITEM_UNITS_PER_LITRE, not the storage scale, because vehicle
    cargo is authored on a different scale than worn containers. Dimensions are
    real centimetres, sorted longest first, which is the form vehicle gates like
    the Humvee's `MaxItemSize 50 200 50` are written in: a long flat launcher box
    slides into a jeep, a fat equipment box needs a truck.

    A crate costs less room than it offers, so hauling one beats hauling loose
    items. That cannot be exploited by nesting - every crate's dimensions exceed
    every crate's MaxItemSize, so no crate fits inside another, or in a backpack.
    """
    litres = outer_litres(extents)
    volume = int(round(litres * ITEM_UNITS_PER_LITRE, -2))
    dimensions = [min(200, round(axis * 100)) for axis in sorted(extents, reverse=True)]
    slot = next(
        (slot for key, slot in SLOT_OVERRIDES.items() if key in name),
        next((slot for cap, slot in SLOT_TIERS if litres < cap), LARGEST_SLOT),
    )
    weight = max(1, round(litres ** (2 / 3) * TARE_WEIGHT_COEFFICIENT))
    return Item(volume, " ".join(str(d) for d in dimensions), slot, weight)


def fits_in_truck(volume, dimensions):
    """Every crate has to be loadable into a transport truck."""
    sizes = sorted((int(d) for d in dimensions.split()), reverse=True)
    return volume <= TRUCK_CARGO_VOLUME and all(
        size <= gate for size, gate in zip(sizes, TRUCK_MAX_ITEM_SIZE)
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--write", action="store_true", help="apply to the prefabs")
    args = parser.parse_args()

    for prefab in sorted(ROOT.glob("PrefabsEditable/**/*.et")):
        extents = crate_extents(prefab)
        if not extents:
            continue

        volume, item_size = limits(extents)
        item = as_item(extents, prefab.stem)
        if not fits_in_truck(item.volume, item.dimensions):
            raise ValueError(
                f"{prefab.name}: costs {item.volume} at {item.dimensions} cm, "
                f"which no transport truck can take ({TRUCK_CARGO_VOLUME} at "
                f"{TRUCK_MAX_ITEM_SIZE}). Lower ITEM_UNITS_PER_LITRE."
            )
        print(
            f"{extents[0]:5.2f} x {extents[1]:5.2f} x {extents[2]:5.2f} m"
            f"  holds {volume:>7} ({item_size})"
            f"  costs {item.volume:>7} ({item.dimensions:>11}, {item.slot})"
            f"  {item.weight:>4} kg  {prefab.stem}"
        )
        if not args.write:
            continue

        text = prefab.read_text()
        edits = [
            (
                r"( *)(?:UseCapacityCoefficient \d+\n *)?MaxCumulativeVolume \d+",
                lambda m: f"{m.group(1)}UseCapacityCoefficient 0\n"
                f"{m.group(1)}MaxCumulativeVolume {volume}",
            ),
            (r"MaxItemSize [\d ]+", f"MaxItemSize {item_size}"),
            (r"ItemDimensions [\d ]+", f"ItemDimensions {item.dimensions}"),
            (r"ItemVolume \d+", f"ItemVolume {item.volume}"),
            (r"m_Size (?:SLOT_[\dx]+|\d+)", f"m_Size {item.slot}"),
            # Lookbehind so this does not fire inside `m_fMaxWeight`.
            (r"(?<![A-Za-z])Weight [\d.]+", f"Weight {item.weight}"),
        ]
        for pattern, replacement in edits:
            text, hits = re.subn(pattern, replacement, text)
            if hits != 1:
                raise ValueError(
                    f"{prefab.name}: {pattern!r} matched {hits} times, expected 1"
                )
        prefab.write_text(text)


def demo():
    """Self-check: the wooden weapon crate is the 48-inch rifle box."""
    extents = crate_extents(
        ROOT / "PrefabsEditable/Auto/Props/Military/AmmoBoxes/US"
        "/E_EquipmentBoxWooden_Weapon_01_US.et"
    )
    assert abs(extents[0] - 1.219) < 0.01, extents
    volume, item_size = limits(extents)
    assert item_size == BIG_ITEM_SIZE, "a rifle crate must accept rifles"
    assert 50000 < volume < 60000, volume

    can = crate_extents(
        ROOT / "PrefabsEditable/Auto/Props/Military/AmmoBoxes"
        "/E_AmmoBox_50cal_100rnd.et"
    )
    assert limits(can)[1] == SMALL_ITEM_SIZE, "a .50 cal can must refuse rifles"
    assert limits(can)[0] < volume / 10, "a can must hold far less than a crate"

    # A can costs far less to haul than a crate, and dimensions come out
    # longest-first in real centimetres.
    crate, tin = as_item(extents), as_item(can)
    assert tin.volume < crate.volume / 10, (tin.volume, crate.volume)
    assert crate.dimensions == "122 57 40", crate.dimensions
    assert (tin.slot, crate.slot) == ("SLOT_1x1", "SLOT_3x3"), (tin.slot, crate.slot)
    assert fits_in_truck(crate.volume, crate.dimensions)
    assert fits_in_truck(tin.volume, tin.dimensions)

    # Tare weight against the real articles: a 48-inch wooden ammo crate is
    # about 20 kg empty, an M2A1 .50 cal can about 2.7 kg. The shell law has to
    # keep the can far heavier per litre than the crate.
    assert crate.weight == 20, crate.weight
    assert tin.weight == 2, tin.weight
    assert tin.weight / outer_litres(can) > crate.weight / outer_litres(extents)

    every = {
        p.stem: as_item(e, p.stem)
        for p in ROOT.glob("PrefabsEditable/**/*.et")
        for e in [crate_extents(p)]
        if e
    }

    # The bulkiest crate in the mod still has to fit, with room for a second one.
    biggest = max(every.values(), key=lambda item: item.volume)
    assert fits_in_truck(biggest.volume, biggest.dimensions), biggest
    assert biggest.volume * 2 <= TRUCK_CARGO_VOLUME, biggest

    # Cargo volume has to stay the binding limit everywhere, so that scaling
    # weight never quietly changes how much a vehicle can take. The tightest
    # vehicles are the jeeps: 50000 units against 400 kg.
    assert all(
        item.volume / 50000 > item.weight / 400 for item in every.values()
    ), "weight now binds before volume in a jeep"

    # Hand-set stack footprints beat the volume tiers, covered twins included.
    for stem, item in every.items():
        for key, wanted in SLOT_OVERRIDES.items():
            if key in stem:
                assert item.slot == wanted, (stem, item.slot, wanted)
    assert every["E_EquipmentBoxStack_US_01_V1_covered"].slot == "SLOT_2x2"
    assert every["E_EquipmentBoxStack_USSR_01_V3"].slot == "SLOT_3x3"
    print("demo ok")


if __name__ == "__main__":
    main()
