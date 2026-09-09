# Logistics server configuration

Open `Configs/Server/ServerConfig.conf` in Workbench and add crate rules to `m_aCrates`.

Each rule contains:

- `m_sCratePrefab`: an InventoryBoxes crate prefab.
- `m_sAllowedItems`: optional `MaxCount=ItemPrefab;MaxCount=ItemPrefab` entries.

When no rules exist, all InventoryBoxes crates and all arsenal request contents remain available. Once at least one rule exists, only configured crates can be created. An empty `m_sAllowedItems` allows any requested arsenal content in that crate; otherwise every requested item must be listed and its quantity must not exceed the configured maximum.
