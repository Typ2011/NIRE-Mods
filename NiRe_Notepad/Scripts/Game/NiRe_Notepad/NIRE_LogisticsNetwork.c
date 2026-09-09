enum NIRE_ELogisticsRequestStatus
{
	OPEN,
	ACCEPTED,
	HOLD,
	REJECTED,
	READY,
	DELIVERY_IN_TRANSIT,
	DELIVERY_ARRIVED,
	PICKUP_READY,
	COMPLETED
}

enum NIRE_ELogisticsDeliveryMode
{
	PICKUP,
	DELIVERY
}

enum NIRE_ELogisticsRole
{
	NONE,
	REQUESTER,
	LOGISTICIAN
}

class NIRE_LogisticsRequest
{
	int m_iId;
	int m_iRequesterPlayerId;
	int m_iHandlerPlayerId = -1;
	string m_sFactionKey;
	string m_sMaterialData;
	string m_sMaterialName;
	int m_iQuantity;
	ref array<ResourceName> m_aMaterialPrefabs = {};
	ref array<string> m_aMaterialNames = {};
	ref array<int> m_aMaterialQuantities = {};
	NIRE_ELogisticsDeliveryMode m_eDeliveryMode;
	string m_sCoordinate;
	string m_sNote;
	NIRE_ELogisticsRequestStatus m_eStatus;
}

modded class SCR_PlayerController
{
	protected static const int NIRE_LOGISTICS_MAX_QUANTITY = 999;
	protected static const int NIRE_LOGISTICS_MAX_ITEM_TYPES = 100;
	protected static const int NIRE_LOGISTICS_MAX_TOTAL_ITEMS = 10000;
	protected static ref array<ref NIRE_LogisticsRequest> s_aNIRE_ServerRequests = {};
	protected static ref map<int, NIRE_ELogisticsRole> s_mNIRE_LogisticsRoles = new map<int, NIRE_ELogisticsRole>();
	protected static int s_iNIRE_NextRequestId = 1;
	protected ref array<ref NIRE_LogisticsRequest> m_aNIRE_LogisticsRequests = {};
	protected NIRE_ELogisticsRole m_eNIRE_LogisticsRole;
	protected ref array<int> m_aNIRE_LogisticsAccessPlayerIds = {};
	protected ref array<NIRE_ELogisticsRole> m_aNIRE_LogisticsAccessRoles = {};

	array<ref NIRE_LogisticsRequest> NIRE_GetLogisticsRequests()
	{
		return m_aNIRE_LogisticsRequests;
	}

	bool NIRE_HasLogisticsAccess()
	{
		return NIRE_GetLocalLogisticsRole() != NIRE_ELogisticsRole.NONE;
	}

	bool NIRE_IsLogistician()
	{
		return NIRE_GetLocalLogisticsRole() == NIRE_ELogisticsRole.LOGISTICIAN;
	}

	NIRE_ELogisticsRole NIRE_GetLocalLogisticsRole()
	{
		if (NIRE_HasPermanentLogisticsAccess(SCR_PlayerController.GetLocalPlayerId()))
			return NIRE_ELogisticsRole.LOGISTICIAN;

		return m_eNIRE_LogisticsRole;
	}

	NIRE_ELogisticsRole NIRE_GetPlayerLogisticsRole(int playerId)
	{
		if (NIRE_HasPermanentLogisticsAccess(playerId))
			return NIRE_ELogisticsRole.LOGISTICIAN;

		int index = m_aNIRE_LogisticsAccessPlayerIds.Find(playerId);
		if (index < 0)
			return NIRE_ELogisticsRole.NONE;

		return m_aNIRE_LogisticsAccessRoles[index];
	}

	bool NIRE_IsPlayerLogisticsPermanent(int playerId)
	{
		return NIRE_HasPermanentLogisticsAccess(playerId);
	}

	void NIRE_RequestLogisticsAccessSnapshot()
	{
		m_aNIRE_LogisticsAccessPlayerIds.Clear();
		m_aNIRE_LogisticsAccessRoles.Clear();
		Rpc(RpcAsk_NIRE_LogisticsAccessSnapshot);
	}

	void NIRE_SetLogisticsRole(int playerId, NIRE_ELogisticsRole role)
	{
		Rpc(RpcAsk_NIRE_SetLogisticsRole, playerId, role);
	}

	void NIRE_RequestLogisticsSnapshot()
	{
		m_aNIRE_LogisticsRequests.Clear();
		Rpc(RpcAsk_NIRE_LogisticsSnapshot);
	}

	void NIRE_SubmitLogisticsRequest(int requestId, string materialData, NIRE_ELogisticsDeliveryMode deliveryMode, string coordinate, string note)
	{
		Rpc(RpcAsk_NIRE_SubmitLogisticsRequest, requestId, materialData, deliveryMode, coordinate, note);
	}

	void NIRE_ManageLogisticsRequest(int requestId, NIRE_ELogisticsRequestStatus status)
	{
		Rpc(RpcAsk_NIRE_ManageLogisticsRequest, requestId, status);
	}

	void NIRE_CreateLogisticsCrate(int requestId, ResourceName cratePrefab)
	{
		Rpc(RpcAsk_NIRE_CreateLogisticsCrate, requestId, cratePrefab);
	}

	protected static bool NIRE_IsGameMaster(int playerId)
	{
		SCR_EditorManagerCore editorCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		SCR_EditorManagerEntity editorManager;
		if (editorCore)
			editorManager = editorCore.GetEditorManager(playerId);
		return editorManager && !editorManager.IsLimited();
	}

	protected static bool NIRE_HasPermanentLogisticsAccess(int playerId)
	{
		return playerId > 0 && (SCR_Global.IsAdmin(playerId) || NIRE_IsGameMaster(playerId));
	}

	protected static bool NIRE_HasServerLogisticsAccess(int playerId)
	{
		return NIRE_GetServerLogisticsRole(playerId) != NIRE_ELogisticsRole.NONE;
	}

	protected static bool NIRE_HasServerLogisticianAccess(int playerId)
	{
		return NIRE_GetServerLogisticsRole(playerId) == NIRE_ELogisticsRole.LOGISTICIAN;
	}

	protected static NIRE_ELogisticsRole NIRE_GetServerLogisticsRole(int playerId)
	{
		if (NIRE_HasPermanentLogisticsAccess(playerId))
			return NIRE_ELogisticsRole.LOGISTICIAN;

		NIRE_ELogisticsRole role;
		if (s_mNIRE_LogisticsRoles.Find(playerId, role))
			return role;

		return NIRE_ELogisticsRole.NONE;
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_NIRE_LogisticsSnapshot()
	{
		int playerId = NIRE_GetOwnerPlayerId();
		if (!NIRE_HasServerLogisticsAccess(playerId))
			return;

		string factionKey = NIRE_GetPlayerFactionKey(playerId);
		if (factionKey.IsEmpty())
			return;

		foreach (NIRE_LogisticsRequest request : s_aNIRE_ServerRequests)
		{
			if (request.m_sFactionKey == factionKey && NIRE_CanReceiveLogisticsRequest(playerId, request))
				NIRE_SendLogisticsRequest(request);
		}

		Rpc(RpcDo_NIRE_LogisticsSnapshotDone);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_NIRE_SubmitLogisticsRequest(int requestId, string materialData, NIRE_ELogisticsDeliveryMode deliveryMode, string coordinate, string note)
	{
		int playerId = NIRE_GetOwnerPlayerId();
		if (!NIRE_HasServerLogisticsAccess(playerId))
			return;

		string factionKey = NIRE_GetPlayerFactionKey(playerId);
		coordinate = coordinate.Trim();
		note = note.Trim();
		if (factionKey.IsEmpty() || materialData.Length() > 32768 || coordinate.Length() > 64 || note.Length() > 1024)
			return;
		if (deliveryMode == NIRE_ELogisticsDeliveryMode.DELIVERY && coordinate.IsEmpty())
			return;

		array<ResourceName> materialPrefabs = {};
		array<int> materialCounts = {};
		int totalItems;
		if (!NIRE_ParseLogisticsMaterials(materialData, materialPrefabs, materialCounts, totalItems))
			return;
		NIRE_LogisticsServerConfig serverConfig = NIRE_LogisticsServerConfig.Load();
		if (serverConfig && !serverConfig.AllowsAnyCrateContents(materialPrefabs, materialCounts))
			return;

		NIRE_LogisticsRequest request;
		NIRE_ELogisticsRequestStatus previousStatus;
		bool created = requestId <= 0;
		if (requestId > 0)
		{
			request = NIRE_FindServerLogisticsRequest(requestId);
			if (!NIRE_HasServerLogisticianAccess(playerId) || !request || request.m_sFactionKey != factionKey)
				return;
			previousStatus = request.m_eStatus;
		}
		else
		{
			request = new NIRE_LogisticsRequest();
			request.m_iId = s_iNIRE_NextRequestId++;
			request.m_iRequesterPlayerId = playerId;
			request.m_sFactionKey = factionKey;
			s_aNIRE_ServerRequests.Insert(request);
		}

		request.m_sMaterialData = materialData;
		request.m_iQuantity = totalItems;
		if (request.m_eStatus == NIRE_ELogisticsRequestStatus.PICKUP_READY && deliveryMode == NIRE_ELogisticsDeliveryMode.DELIVERY)
			request.m_eStatus = NIRE_ELogisticsRequestStatus.READY;
		else if (deliveryMode == NIRE_ELogisticsDeliveryMode.PICKUP)
		{
			if (request.m_eStatus == NIRE_ELogisticsRequestStatus.READY || request.m_eStatus == NIRE_ELogisticsRequestStatus.DELIVERY_IN_TRANSIT)
				request.m_eStatus = NIRE_ELogisticsRequestStatus.PICKUP_READY;
			else if (request.m_eStatus == NIRE_ELogisticsRequestStatus.DELIVERY_ARRIVED)
				request.m_eStatus = NIRE_ELogisticsRequestStatus.PICKUP_READY;
		}
		request.m_eDeliveryMode = deliveryMode;
		request.m_sCoordinate = coordinate;
		request.m_sNote = note;
		NIRE_BroadcastLogisticsRequest(request);
		if (created || previousStatus != request.m_eStatus)
			NIRE_BroadcastLogisticsStatus(request);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_NIRE_ManageLogisticsRequest(int requestId, NIRE_ELogisticsRequestStatus status)
	{
		int playerId = NIRE_GetOwnerPlayerId();
		if (!NIRE_HasServerLogisticianAccess(playerId))
			return;

		NIRE_LogisticsRequest request = NIRE_FindServerLogisticsRequest(requestId);
		if (!request || request.m_sFactionKey != NIRE_GetPlayerFactionKey(playerId) || !NIRE_CanSetLogisticsStatus(request, status))
			return;

		request.m_iHandlerPlayerId = playerId;
		request.m_eStatus = status;
		NIRE_BroadcastLogisticsRequest(request);
		NIRE_BroadcastLogisticsStatus(request);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_NIRE_CreateLogisticsCrate(int requestId, ResourceName cratePrefab)
	{
		int playerId = NIRE_GetOwnerPlayerId();
		NIRE_LogisticsRequest request = NIRE_FindServerLogisticsRequest(requestId);
		if (!NIRE_HasServerLogisticianAccess(playerId) || !request || request.m_sFactionKey != NIRE_GetPlayerFactionKey(playerId) || request.m_eStatus != NIRE_ELogisticsRequestStatus.ACCEPTED)
			return;

		array<ResourceName> materialPrefabs = {};
		array<int> materialCounts = {};
		int totalItems;
		if (!NIRE_ParseLogisticsMaterials(request.m_sMaterialData, materialPrefabs, materialCounts, totalItems))
			return;

		NIRE_LogisticsServerConfig serverConfig = NIRE_LogisticsServerConfig.Load();
		if (serverConfig && !serverConfig.AllowsCrateContents(cratePrefab, materialPrefabs, materialCounts))
			return;

		Resource resource = Resource.Load(cratePrefab);
		if (!resource || !resource.IsValid() || !SCR_BaseContainerTools.FindComponentSource(resource, IBX_GMInventoryEditorComponent))
			return;

		IEntity player = NIRE_GetPlayerEntity(playerId);
		if (!player)
			return;

		BaseWorld world = player.GetWorld();
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		player.GetWorldTransform(spawnParams.Transform);
		spawnParams.TransformMode = ETransformMode.WORLD;
		float yaw = player.GetYawPitchRoll()[0] * Math.DEG2RAD;
		vector spawnPosition = player.GetOrigin() + Vector(Math.Sin(yaw) * 2, 0, Math.Cos(yaw) * 2);
		spawnPosition[1] = world.GetSurfaceY(spawnPosition[0], spawnPosition[2]);
		spawnParams.Transform[3] = spawnPosition;
		IEntity crate = GetGame().SpawnEntityPrefab(resource, world, spawnParams);
		if (!crate)
			return;

		IBX_GMInventoryEditorComponent inventoryEditor = IBX_GMInventoryEditorComponent.Cast(crate.FindComponent(IBX_GMInventoryEditorComponent));
		if (!inventoryEditor)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(crate);
			return;
		}

		if (!NIRE_FillLogisticsCrate(crate, materialPrefabs, materialCounts))
		{
			SCR_EntityHelper.DeleteEntityAndChildren(crate);
			return;
		}

		request.m_iHandlerPlayerId = playerId;
		if (request.m_eDeliveryMode == NIRE_ELogisticsDeliveryMode.DELIVERY)
			request.m_eStatus = NIRE_ELogisticsRequestStatus.READY;
		else
			request.m_eStatus = NIRE_ELogisticsRequestStatus.PICKUP_READY;
		NIRE_BroadcastLogisticsRequest(request);
		NIRE_BroadcastLogisticsStatus(request);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_NIRE_LogisticsAccessSnapshot()
	{
		int playerId = NIRE_GetOwnerPlayerId();
		if (!NIRE_HasPermanentLogisticsAccess(playerId))
			return;

		PlayerManager players = GetGame().GetPlayerManager();
		array<int> playerIds = {};
		players.GetPlayers(playerIds);
		foreach (int targetPlayerId : playerIds)
			Rpc(RpcDo_NIRE_LogisticsAccessEntry, targetPlayerId, NIRE_GetServerLogisticsRole(targetPlayerId));
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_NIRE_SetLogisticsRole(int targetPlayerId, NIRE_ELogisticsRole role)
	{
		if (!NIRE_HasPermanentLogisticsAccess(NIRE_GetOwnerPlayerId()) || role < NIRE_ELogisticsRole.NONE || role > NIRE_ELogisticsRole.LOGISTICIAN)
			return;

		PlayerManager players = GetGame().GetPlayerManager();
		SCR_PlayerController target = SCR_PlayerController.Cast(players.GetPlayerController(targetPlayerId));
		if (!target)
			return;

		if (NIRE_HasPermanentLogisticsAccess(targetPlayerId))
			role = NIRE_ELogisticsRole.LOGISTICIAN;
		if (role == NIRE_ELogisticsRole.NONE)
			s_mNIRE_LogisticsRoles.Remove(targetPlayerId);
		else
			s_mNIRE_LogisticsRoles.Set(targetPlayerId, role);

		target.NIRE_SendOwnLogisticsRole(role);
		NIRE_BroadcastLogisticsAccessEntry(targetPlayerId, role);
	}

	protected void NIRE_SendOwnLogisticsRole(NIRE_ELogisticsRole role)
	{
		Rpc(RpcDo_NIRE_OwnLogisticsRole, role);
	}

	protected void NIRE_SendLogisticsAccessEntry(int playerId, NIRE_ELogisticsRole role)
	{
		Rpc(RpcDo_NIRE_LogisticsAccessEntry, playerId, role);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_NIRE_OwnLogisticsRole(NIRE_ELogisticsRole role)
	{
		m_eNIRE_LogisticsRole = role;
		if (role != NIRE_ELogisticsRole.NONE)
			NIRE_RequestLogisticsSnapshot();
		else
		{
			m_aNIRE_LogisticsRequests.Clear();
			NIRE_LogisticsScreen.RefreshIfOpen();
		}
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_NIRE_LogisticsAccessEntry(int playerId, NIRE_ELogisticsRole role)
	{
		int index = m_aNIRE_LogisticsAccessPlayerIds.Find(playerId);
		if (index < 0)
		{
			m_aNIRE_LogisticsAccessPlayerIds.Insert(playerId);
			m_aNIRE_LogisticsAccessRoles.Insert(role);
		}
		else
			m_aNIRE_LogisticsAccessRoles[index] = role;
	}

	static void NIRE_RemoveLogisticsAccess(int playerId)
	{
		if (!Replication.IsServer() || !s_mNIRE_LogisticsRoles.Contains(playerId))
			return;

		s_mNIRE_LogisticsRoles.Remove(playerId);
		NIRE_BroadcastLogisticsAccessEntry(playerId, NIRE_ELogisticsRole.NONE);
	}

	protected void NIRE_SendLogisticsRequest(notnull NIRE_LogisticsRequest request)
	{
		Rpc(RpcDo_NIRE_UpsertLogisticsRequestHeader, request.m_iId, request.m_iRequesterPlayerId, request.m_iHandlerPlayerId, request.m_sFactionKey, request.m_sMaterialData, request.m_iQuantity);
		Rpc(RpcDo_NIRE_UpsertLogisticsRequestDetails, request.m_iId, request.m_eDeliveryMode, request.m_sCoordinate, request.m_sNote, request.m_eStatus);
	}

	protected void NIRE_SendLogisticsStatus(notnull NIRE_LogisticsRequest request)
	{
		Rpc(RpcDo_NIRE_LogisticsStatus, request.m_iId, request.m_eStatus);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_NIRE_LogisticsStatus(int requestId, NIRE_ELogisticsRequestStatus status)
	{
		SCR_ChatPanelManager chat = SCR_ChatPanelManager.GetInstance();
		if (!chat)
			return;

		string statusText = WidgetManager.Translate(NIRE_GetLogisticsStatusLabel(status));
		string message = WidgetManager.Translate("#NIRE-Logistics_StatusNotification", string.Format("%1", requestId), statusText);
		chat.ShowHelpMessage("[NiRe Logistics] " + message);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_NIRE_UpsertLogisticsRequestHeader(int id, int requesterId, int handlerId, string factionKey, string materialData, int quantity)
	{
		NIRE_LogisticsRequest request = NIRE_GetOrCreateClientLogisticsRequest(id);
		request.m_iRequesterPlayerId = requesterId;
		request.m_iHandlerPlayerId = handlerId;
		request.m_sFactionKey = factionKey;
		request.m_sMaterialData = materialData;
		int totalItems;
		NIRE_ParseLogisticsMaterials(materialData, request.m_aMaterialPrefabs, request.m_aMaterialQuantities, totalItems);
		request.m_aMaterialNames.Clear();
		request.m_sMaterialName = string.Empty;
		foreach (int index, ResourceName prefab : request.m_aMaterialPrefabs)
		{
			string displayName;
			NIRE_FindArsenalItem(prefab, displayName);
			displayName = WidgetManager.Translate(displayName);
			request.m_aMaterialNames.Insert(displayName);
			if (!request.m_sMaterialName.IsEmpty())
				request.m_sMaterialName += ", ";
			request.m_sMaterialName += string.Format("%1 x%2", displayName, request.m_aMaterialQuantities[index]);
		}
		request.m_iQuantity = quantity;
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_NIRE_UpsertLogisticsRequestDetails(int id, NIRE_ELogisticsDeliveryMode deliveryMode, string coordinate, string note, NIRE_ELogisticsRequestStatus status)
	{
		NIRE_LogisticsRequest request = NIRE_GetOrCreateClientLogisticsRequest(id);
		request.m_eDeliveryMode = deliveryMode;
		request.m_sCoordinate = coordinate;
		request.m_sNote = note;
		request.m_eStatus = status;
		NIRE_LogisticsScreen.RefreshIfOpen();
	}

	protected NIRE_LogisticsRequest NIRE_GetOrCreateClientLogisticsRequest(int id)
	{
		foreach (NIRE_LogisticsRequest request : m_aNIRE_LogisticsRequests)
		{
			if (request.m_iId == id)
				return request;
		}

		NIRE_LogisticsRequest request = new NIRE_LogisticsRequest();
		request.m_iId = id;
		m_aNIRE_LogisticsRequests.Insert(request);
		return request;
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_NIRE_LogisticsSnapshotDone()
	{
		NIRE_LogisticsScreen.RefreshIfOpen();
	}

	protected static NIRE_LogisticsRequest NIRE_FindServerLogisticsRequest(int requestId)
	{
		foreach (NIRE_LogisticsRequest request : s_aNIRE_ServerRequests)
		{
			if (request.m_iId == requestId)
				return request;
		}

		return null;
	}

	protected static bool NIRE_CanSetLogisticsStatus(notnull NIRE_LogisticsRequest request, NIRE_ELogisticsRequestStatus status)
	{
		if (request.m_eStatus == NIRE_ELogisticsRequestStatus.COMPLETED)
			return false;

		if (status == NIRE_ELogisticsRequestStatus.ACCEPTED || status == NIRE_ELogisticsRequestStatus.HOLD || status == NIRE_ELogisticsRequestStatus.REJECTED)
			return request.m_eStatus != NIRE_ELogisticsRequestStatus.READY && request.m_eStatus != NIRE_ELogisticsRequestStatus.DELIVERY_IN_TRANSIT && request.m_eStatus != NIRE_ELogisticsRequestStatus.DELIVERY_ARRIVED && request.m_eStatus != NIRE_ELogisticsRequestStatus.PICKUP_READY;
		if (status == NIRE_ELogisticsRequestStatus.DELIVERY_IN_TRANSIT)
			return request.m_eDeliveryMode == NIRE_ELogisticsDeliveryMode.DELIVERY && request.m_eStatus == NIRE_ELogisticsRequestStatus.READY;
		if (status == NIRE_ELogisticsRequestStatus.DELIVERY_ARRIVED)
			return request.m_eDeliveryMode == NIRE_ELogisticsDeliveryMode.DELIVERY && request.m_eStatus == NIRE_ELogisticsRequestStatus.DELIVERY_IN_TRANSIT;
		if (status == NIRE_ELogisticsRequestStatus.COMPLETED)
			return request.m_eStatus == NIRE_ELogisticsRequestStatus.DELIVERY_ARRIVED || request.m_eStatus == NIRE_ELogisticsRequestStatus.PICKUP_READY;

		return false;
	}

	protected static bool NIRE_ParseLogisticsMaterials(string materialData, notnull array<ResourceName> prefabs, notnull array<int> counts, out int totalItems)
	{
		prefabs.Clear();
		counts.Clear();
		totalItems = 0;
		array<string> entries = {};
		materialData.Split(";", entries, true);
		set<ResourceName> uniquePrefabs = new set<ResourceName>();
		foreach (string entry : entries)
		{
			array<string> fields = {};
			entry.Split("=", fields, false);
			if (fields.Count() != 2)
				return false;

			int count = fields[0].ToInt();
			ResourceName prefab = fields[1].Trim();
			string displayName;
			if (count < 1 || count > NIRE_LOGISTICS_MAX_QUANTITY || prefab.IsEmpty() || uniquePrefabs.Contains(prefab) || !NIRE_FindArsenalItem(prefab, displayName))
				return false;

			uniquePrefabs.Insert(prefab);
			prefabs.Insert(prefab);
			counts.Insert(count);
			totalItems += count;
		}

		return !prefabs.IsEmpty() && prefabs.Count() <= NIRE_LOGISTICS_MAX_ITEM_TYPES && totalItems <= NIRE_LOGISTICS_MAX_TOTAL_ITEMS;
	}

	protected static bool NIRE_FillLogisticsCrate(notnull IEntity crate, notnull array<ResourceName> prefabs, notnull array<int> counts)
	{
		InventoryStorageManagerComponent manager = InventoryStorageManagerComponent.Cast(crate.FindComponent(InventoryStorageManagerComponent));
		BaseInventoryStorageComponent storage = BaseInventoryStorageComponent.Cast(crate.FindComponent(SCR_UniversalInventoryStorageComponent));
		if (!manager || !storage)
			return false;

		int added;
		foreach (int materialIndex, ResourceName prefab : prefabs)
		{
			Resource itemResource = Resource.Load(prefab);
			if (!itemResource || !itemResource.IsValid())
				return false;
			for (int itemIndex = 0; itemIndex < counts[materialIndex]; itemIndex++)
			{
				IEntity item = GetGame().SpawnEntityPrefab(itemResource, GetGame().GetWorld());
				if (!item)
					break;

				if (manager.TryInsertItemInStorage(item, storage))
				{
					added++;
					continue;
				}

				SCR_EntityHelper.DeleteEntityAndChildren(item);
				break;
			}
		}

		return added > 0;
	}

	protected static bool NIRE_FindArsenalItem(ResourceName prefab, out string displayName)
	{
		if (prefab.IsEmpty())
			return false;

		SCR_EntityCatalogManagerComponent catalog = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!catalog)
			return false;

		array<SCR_ArsenalItem> items = {};
		catalog.GetArsenalItems(items);
		if (NIRE_FindArsenalItemIn(items, prefab, displayName))
			return true;

		array<Faction> factions = {};
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
			factionManager.GetFactionsList(factions);
		foreach (Faction faction : factions)
		{
			SCR_Faction scrFaction = SCR_Faction.Cast(faction);
			if (!scrFaction)
				continue;

			items.Clear();
			if (catalog.GetFactionArsenalItems(items, scrFaction) && NIRE_FindArsenalItemIn(items, prefab, displayName))
				return true;
		}

		return false;
	}

	protected static bool NIRE_FindArsenalItemIn(notnull array<SCR_ArsenalItem> items, ResourceName prefab, out string displayName)
	{
		foreach (SCR_ArsenalItem item : items)
		{
			if (item.GetItemResourceName() != prefab)
				continue;

			displayName = FilePath.StripExtension(FilePath.StripPath(prefab));
			Resource resource = item.GetItemResource();
			if (!resource || !resource.IsValid())
				return true;

			IEntityComponentSource componentSource = SCR_BaseContainerTools.FindComponentSource(resource, InventoryItemComponent);
			SCR_ItemAttributeCollection attributes;
			if (componentSource)
				attributes = SCR_ComponentHelper.GetInventoryItemInfo(componentSource);
			UIInfo info;
			if (attributes)
				info = attributes.GetUIInfo();
			if (info && !info.GetName().IsEmpty())
				displayName = info.GetName();
			return true;
		}

		return false;
	}

	protected static void NIRE_BroadcastLogisticsRequest(notnull NIRE_LogisticsRequest request)
	{
		PlayerManager players = GetGame().GetPlayerManager();
		array<int> playerIds = {};
		players.GetPlayers(playerIds);
		foreach (int playerId : playerIds)
		{
			if (!NIRE_CanReceiveLogisticsRequest(playerId, request))
				continue;

			SCR_PlayerController controller = SCR_PlayerController.Cast(players.GetPlayerController(playerId));
			if (controller)
				controller.NIRE_SendLogisticsRequest(request);
		}
	}

	protected static void NIRE_BroadcastLogisticsStatus(notnull NIRE_LogisticsRequest request)
	{
		PlayerManager players = GetGame().GetPlayerManager();
		array<int> playerIds = {};
		players.GetPlayers(playerIds);
		foreach (int playerId : playerIds)
		{
			if (!NIRE_CanReceiveLogisticsRequest(playerId, request))
				continue;

			SCR_PlayerController controller = SCR_PlayerController.Cast(players.GetPlayerController(playerId));
			if (controller)
				controller.NIRE_SendLogisticsStatus(request);
		}
	}

	protected static bool NIRE_CanReceiveLogisticsRequest(int playerId, notnull NIRE_LogisticsRequest request)
	{
		if (NIRE_GetPlayerFactionKey(playerId) != request.m_sFactionKey)
			return false;

		NIRE_ELogisticsRole role = NIRE_GetServerLogisticsRole(playerId);
		return role == NIRE_ELogisticsRole.LOGISTICIAN || role == NIRE_ELogisticsRole.REQUESTER && playerId == request.m_iRequesterPlayerId;
	}

	protected static void NIRE_BroadcastLogisticsAccessEntry(int targetPlayerId, NIRE_ELogisticsRole role)
	{
		PlayerManager players = GetGame().GetPlayerManager();
		array<int> playerIds = {};
		players.GetPlayers(playerIds);
		foreach (int playerId : playerIds)
		{
			if (!NIRE_HasPermanentLogisticsAccess(playerId))
				continue;

			SCR_PlayerController controller = SCR_PlayerController.Cast(players.GetPlayerController(playerId));
			if (controller)
				controller.NIRE_SendLogisticsAccessEntry(targetPlayerId, role);
		}
	}

	protected static LocalizedString NIRE_GetLogisticsStatusLabel(NIRE_ELogisticsRequestStatus status)
	{
		switch (status)
		{
			case NIRE_ELogisticsRequestStatus.ACCEPTED: return "#NIRE-Logistics_StatusAccepted";
			case NIRE_ELogisticsRequestStatus.HOLD: return "#NIRE-Logistics_StatusHold";
			case NIRE_ELogisticsRequestStatus.REJECTED: return "#NIRE-Logistics_StatusRejected";
			case NIRE_ELogisticsRequestStatus.READY: return "#NIRE-Logistics_StatusReady";
			case NIRE_ELogisticsRequestStatus.DELIVERY_IN_TRANSIT: return "#NIRE-Logistics_StatusDeliveryInTransit";
			case NIRE_ELogisticsRequestStatus.DELIVERY_ARRIVED: return "#NIRE-Logistics_StatusDeliveryArrived";
			case NIRE_ELogisticsRequestStatus.PICKUP_READY: return "#NIRE-Logistics_StatusPickupReady";
			case NIRE_ELogisticsRequestStatus.COMPLETED: return "#NIRE-Logistics_StatusCompleted";
		}

		return "#NIRE-Logistics_StatusOpen";
	}

	protected int NIRE_GetOwnerPlayerId()
	{
		PlayerManager players = GetGame().GetPlayerManager();
		array<int> playerIds = {};
		players.GetPlayers(playerIds);
		foreach (int playerId : playerIds)
		{
			if (players.GetPlayerController(playerId) == this)
				return playerId;
		}

		return 0;
	}

	protected static string NIRE_GetPlayerFactionKey(int playerId)
	{
		IEntity player = NIRE_GetPlayerEntity(playerId);
		FactionAffiliationComponent affiliation;
		if (player)
			affiliation = FactionAffiliationComponent.Cast(player.FindComponent(FactionAffiliationComponent));
		if (!affiliation)
			return string.Empty;

		return affiliation.GetAffiliatedFactionKey();
	}

	protected static IEntity NIRE_GetPlayerEntity(int playerId)
	{
		PlayerController controller = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!controller)
			return null;

		return controller.GetControlledEntity();
	}
}
