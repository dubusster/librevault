/* Copyright (C) 2016 Alexander Shishenko <alex@shishenko.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
#include "TrashArchive.h"
#include "control/FolderParams.h"
#include "folder/PathNormalizer.h"
#include "util/conv_fspath.h"
#include <QDir>
#include <QTimer>
#include <boost/filesystem.hpp>

namespace librevault {

TrashArchive::TrashArchive(const FolderParams& params, PathNormalizer* path_normalizer, QObject* parent) :
	ArchiveStrategy(parent),
	params_(params),
	path_normalizer_(path_normalizer),
	archive_path_(params_.system_path + "/archive") {

	QDir().mkpath(archive_path_);
	QTimer::singleShot(10*1000*60, this, &TrashArchive::maintain_cleanup); // Start after a small delay.
}

void TrashArchive::maintain_cleanup() {
	qInfo() << "Starting archive cleanup";

	std::list<boost::filesystem::path> removed_paths;
	try {
		for(auto it = boost::filesystem::recursive_directory_iterator(conv_fspath(archive_path_)); it != boost::filesystem::recursive_directory_iterator(); it++) {
			time_t time_since_archivation = time(nullptr) - boost::filesystem::last_write_time(it->path());
			constexpr unsigned sec_per_day = 60*60*24;
			if(time_since_archivation >= params_.archive_trash_ttl * sec_per_day && params_.archive_trash_ttl != 0)
				removed_paths.push_back(it->path());
		}

		for(const boost::filesystem::path& path : removed_paths)
			boost::filesystem::remove(path);

		QTimer::singleShot(24*1000*60*60, this, &TrashArchive::maintain_cleanup); // A day.
	}catch(std::exception& e) {
		QTimer::singleShot(10*1000*60, this, &TrashArchive::maintain_cleanup);   // An error occured, retry in 10 min
	}
}

void TrashArchive::archive(QString denormpath) {
	QString archived_path = archive_path_ + "/" + path_normalizer_->normalizePath(denormpath);
	qDebug() << "Adding an archive item: " << archived_path;
	QFile::rename(denormpath, archived_path);
	boost::filesystem::last_write_time(conv_fspath(archived_path), time(nullptr));
}

} /* namespace librevault */
