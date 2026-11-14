from __future__ import annotations

import asyncio
import logging

import voluptuous as vol
from aioesphomeapi import APIClient, TextSensorInfo

from homeassistant import config_entries
from homeassistant.const import CONF_HOST, CONF_PORT
from homeassistant.data_entry_flow import FlowResult

from .const import CONF_NOISE_PSK, DEFAULT_PORT, DOMAIN, TOPOLOGY_NAME

_LOGGER = logging.getLogger(__name__)


class WavinAHC9000ConfigFlow(config_entries.ConfigFlow, domain=DOMAIN):
    """Configure a Wavin AHC9000 ESPHome device."""

    VERSION = 1

    async def _async_probe(self, host: str, port: int, noise_psk: str | None):
        client = APIClient(host, port, None, noise_psk=noise_psk or None)
        try:
            async with asyncio.timeout(20):
                await client.connect(login=True)
                entities, _services = await client.list_entities_services()
                device_info = await client.device_info()
                if device_info is None:
                    raise RuntimeError("ESPHome device information is unavailable")
                topology_found = any(
                    isinstance(info, TextSensorInfo)
                    and (
                        info.name == TOPOLOGY_NAME
                        or "topology" in (getattr(info, "object_id", "") or "").lower()
                    )
                    for info in entities
                )
                return device_info.mac_address, device_info.name, topology_found
        finally:
            await client.disconnect()

    async def async_step_user(self, user_input=None) -> FlowResult:
        errors: dict[str, str] = {}
        if user_input is not None:
            host = user_input[CONF_HOST]
            port = user_input[CONF_PORT]
            noise_psk = user_input.get(CONF_NOISE_PSK, "").strip() or None
            try:
                mac, name, topology_found = await self._async_probe(host, port, noise_psk)
            except Exception:  # noqa: BLE001
                _LOGGER.exception("Unable to connect to ESPHome device at %s:%s", host, port)
                errors["base"] = "cannot_connect"
            else:
                if not topology_found:
                    errors["base"] = "no_topology"
                else:
                    await self.async_set_unique_id(mac)
                    self._abort_if_unique_id_configured()
                    return self.async_create_entry(
                        title=name or f"Wavin AHC9000 ({mac})",
                        data={
                            CONF_HOST: host,
                            CONF_PORT: port,
                            CONF_NOISE_PSK: noise_psk,
                        },
                    )

        return self.async_show_form(
            step_id="user",
            data_schema=vol.Schema(
                {
                    vol.Required(CONF_HOST): str,
                    vol.Required(CONF_PORT, default=DEFAULT_PORT): int,
                    vol.Optional(CONF_NOISE_PSK, default=""): str,
                }
            ),
            errors=errors,
        )
