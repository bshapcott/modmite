//____________________________________________________________________________
// mite - mod microlite - options.h
//
// Copyright 2009 Brad Shapcott -- see end of file for license & contact info

#ifndef options_h
#define options_h

/// - conditional compile flag for features which should be unavailable in
///   a production build
/// - there are certain features intended only for development which make
///   available functionality that may be a security risk in a production
///   environment
/// - actually removing the code from a production build eliminates completely
///   any associated security risk
#define CH_DEV 1

#endif

//_____________________________________________________________________ CONTACT
// mailto:brad at modmite dot com
// http://modmite.com
//_____________________________________________________________________ LICENSE
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//_________________________________________________________________________ EOF
