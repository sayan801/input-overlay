/*************************************************************************
 * This file is part of input-overlay
 * github.con/univrsal/input-overlay
 * Copyright 2020 univrsal <universailp@web.de>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************/

#include "gamepad.hpp"
#include "network.hpp"
#include "uiohook.hpp"
#include "util.hpp"
#include <signal.h>
#include <stdio.h>

/* Catch Application closing */
void sig_int__handler(int signal)
{
	network::network_loop = false;
	gamepad::hook_run_flag = false;
}

void sig_break__handler(int signal)
{
	network::network_loop = false;
	gamepad::hook_run_flag = false;
}

int main(int argc, char **argv)
{
	signal(SIGINT, &sig_int__handler);
	signal(SIGBREAK, &sig_break__handler);

	if (!network::init())
		return util::RET_NETWORK_INIT;

	printf("Network init done.\n");

	if (!util::parse_arguments(argc, argv))
		return util::RET_ARGUMENT_PARSING; /* Invalid arguments */

	if (!util::cfg.monitor_keyboard && !util::cfg.monitor_mouse && !util::cfg.monitor_gamepad) {
		printf("Nothing to monitor!\n");
		return util::RET_NO_HOOKS;
	}

	if (!network::start_connection()) /* Starts a separate network thread */
	{
		network::close();
		return util::RET_CONNECTION;
	}

	/* This will block if uiohook isn't running */
	if (util::cfg.monitor_gamepad) {
		auto threaded = util::cfg.monitor_keyboard || util::cfg.monitor_mouse;
		if (threaded) {
			if (!gamepad::start_pad_hook(true)) {
				printf("Gamepad hook initialization failed!\n");
				return util::RET_GAMEPAD_INIT;
			}
		} else {
			gamepad::start_pad_hook(false);
		}
	}

	if ((util::cfg.monitor_keyboard || util::cfg.monitor_mouse) && !uiohook::init()) {
		printf("uiohook init failed\n");
		return util::RET_UIOHOOK_INIT;
	}

	util::close_all();

	return 0;
}
