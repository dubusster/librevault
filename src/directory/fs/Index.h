/* Copyright (C) 2015 Alexander Shishenko <GamePad64@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include "../../pch.h"
#include "../../contrib/lvsqlite3/SQLiteWrapper.h"
#include "../Abstract.h"

namespace librevault {

class FSDirectory;
class Index {
public:
	using SignedMeta = Meta::SignedMeta;

	class no_such_meta : public std::runtime_error {
	public:
		no_such_meta() : std::runtime_error("Requested Meta not found"){}
	};

	Index(FSDirectory& dir, Session& session);
	virtual ~Index();

	void wipe();

	// FIXME: Check signature!!!
	void put_Meta(const std::list<SignedMeta>& signed_meta_list);
	void put_Meta(const SignedMeta& signed_meta){put_Meta(std::list<SignedMeta>{signed_meta});}

	SignedMeta get_Meta(const blob& path_id);
	std::list<SignedMeta> get_Meta();

	std::list<SignedMeta> containing_block(const blob& encrypted_data_hash);

	SQLiteDB& db() {return *db_;}

private:
	std::shared_ptr<spdlog::logger> log_;
	FSDirectory& dir_;

	std::unique_ptr<SQLiteDB> db_;	// Better use SOCI library ( https://github.com/SOCI/soci ). My "reinvented wheel" isn't stable enough.

	std::list<SignedMeta> get_Meta(std::string sql, std::map<std::string, SQLValue> values = std::map<std::string, SQLValue>());
};

} /* namespace librevault */