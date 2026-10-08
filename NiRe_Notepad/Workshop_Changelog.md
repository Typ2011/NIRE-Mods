NIRE NOTEPAD - FULL CHANGELOG

HOW TO OPEN THE NOTEPAD

1. Press F6 during gameplay or with the map open. The key can be rebound under Settings - Controls - NiRe Notepad.
2. The notepad opens in the lower right corner. The map stays movable and zoomable outside it.
3. Click a writing row to type in it. Clicking outside the writing area gives focus back to the game or the map.
4. Escape closes one layer at a time: the logistics workspace first if it is open, otherwise the notepad.

Personal notes are stored locally on your machine and survive disconnects, reconnects, crashes and restarts. Supply Requests are synchronized by the server to authorized members of your faction.

8 OCTOBER 2026 - LOGISTICS UPDATE

Requests

- Every request now has a required name of up to 32 characters, shown in the shared request list. Older unnamed requests keep their numbered label.
- New Copy button: opens a new editable draft with the selected request's items, quantities, delivery mode, coordinate and note. Submitting creates a separate request.
- New Delete button: needs a second click to confirm, and the server removes the request for every authorized viewer.
- New Request asks for confirmation before discarding a form you have already filled in. Cancel or Escape keeps it.
- The Submit button is hidden for completed requests.
- Coordinates are entered as XXX-XXX: only digits are accepted and the dash is inserted automatically.

Arsenal

- Favorite items with the star at the right edge of each item row. The star button beside search shows only your favorites within the current category and faction filter.
- Hovering an item image shows a larger preview beside the list. In Weapons the preview zooms out so long weapons fit.

Crates

- The crate picker shows each crate's preview image with minus and plus buttons, so one request can be packed into several crates, including several of the same crate (up to 20 per order).
- Optional crate name: every crate of the order gets that name, numbered from 1 when more than one crate is created.
- The picker shows the selected crates' total volume and weight against the request and blocks creation when the contents do not fit. If the items still do not fit after spawning, the crates are removed again instead of leaving items behind.
- Server crate limits now apply per crate rather than per request. Larger amounts are split across more crates.
- Check Crate and its nearby-crate comparison have been removed; the Request Name field takes its place.

Fixes

- The faction filter and crate picker open as centred cards with their own scroll view, so long lists no longer run behind the request fields and every entry can be reached.
- Request rows in the Logistics workspace use larger text without changing the corner notepad.
- The request column buttons are stacked, so longer German labels no longer overlap the arsenal column.
- The coordinate caption lines up with its input box in every language.

9 SEPTEMBER 2026 - INITIAL RELEASE

Notepad

- Six sections: General, CAS, Artillery, Logistics, MEDEVAC and Terrain Baptism.
- Opens during gameplay and on the open map, on a rebindable key, F6 by default.
- Fixed 5-liner and 9-liner templates for CAS and MEDEVAC, and fixed 5-liner and 7-liner templates for Artillery.
- Every template point is an editable prompt and answer pair; Free Text uses ten individual writing rows.
- Named, reusable and deletable entries for CAS, Artillery and MEDEVAC, kept in their own journals.
- Deleting an entry asks for confirmation.
- Notes are saved immediately, with a temporary file and a backup to recover from.
- Localized in all 13 languages Arma Reforger supports, following your game language.

Terrain Baptism

- A persistent drawing canvas with a pen, an eraser, clear all, and five colors.
- The eraser removes stored strokes instead of painting over them in gray.

Logistics

- Supply Requests are shared with your faction and validated by the server.
- The Logistics button opens a full-screen workspace laid out like the Inventory Boxes crate editor: a request column, an arsenal column with categories, search and faction filter, and a request contents column.
- A request holds any number of materials with an individual quantity each, pickup or delivery, a coordinate, four NOTE lines and a workflow status.
- Selecting a weapon in the arsenal expands its compatible ammunition directly below it.
- Separate requester and logistician roles, granted through two Game Master character context actions. Admins and full Game Masters are always authorized.
- Requesters create and read their own requests; logisticians see and manage every request from their faction.
- Every status change is announced in chat to the requester and to all logisticians of that faction.
- Delivery requests track crate ready, in transit, arrived and completed; pickup requests use their own ready for pickup state.
- Check Crate compares a request against the contents of every crate within 5 metres and shows one named card per crate.
- Create Crate spawns a finite Inventory Boxes crate on the ground and fills it from the request. The server configuration decides which crate prefabs are offered and how much of each item a request may hold.

Requirements and Notes

- Requires Mikes UI and Inventory Boxes. Expanded Saline Bags is supported for its 1000 ml and 1500 ml saline items.
- No ACE runtime dependency.
- The templates are fixed working aids. Check them against your unit's SOPs before operational use.
