 /* fre:ac - free audio converter
  * Copyright (C) 2001-2015 Robert Kausch <robert.kausch@freac.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the "GNU General Public License".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#ifndef H_FREAC
#define H_FREAC

#include <smooth.h>

using namespace smooth;
using namespace smooth::GUI;

namespace BonkEnc
{
	class Config;

	abstract class BonkEnc : public GUI::Application
	{
		protected:
			/* Singleton class, therefore protected constructor/destructor
			 */
			static BonkEnc	*instance;

					 BonkEnc();
			virtual		~BonkEnc();

			Config		*currentConfig;
		public:
			static String	 appName;
			static String	 appLongName;
			static String	 version;
			static String	 architecture;
			static String	 shortVersion;
			static String	 cddbVersion;
			static String	 cddbMode;
			static String	 copyright;
			static String	 website;
			static String	 updatePath;
	};
};

#endif
