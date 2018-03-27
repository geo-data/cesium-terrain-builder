#pragma once

#include <sqlite3.h>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <stdexcept>
#include <sstream>

namespace mapbox { 

struct close_db {
    void operator() (sqlite3* db) { if (db) { sqlite3_close(db); }};
};

struct close_stmt {
    void operator() (sqlite3_stmt* stmt) { if (stmt) { sqlite3_finalize(stmt); }};
};

using sqlite_ptr = std::unique_ptr<sqlite3, close_db>;
using sqlite_stmt_ptr = std::unique_ptr<sqlite3_stmt, close_stmt>;

struct sqlite_db {
    sqlite_ptr db;
    sqlite_stmt_ptr tile_stmt;
};

inline sqlite_db mbtiles_open(std::string const& dbname) {
    
    sqlite3 *db;
    if (sqlite3_open(dbname.c_str(), &db) != SQLITE_OK) {
        std::ostringstream err;
        err << "SQLite Error: Failed to open " << dbname << " - " << sqlite3_errmsg(db) << std::endl;
        throw std::runtime_error(err.str());
    }
    
    sqlite_ptr outdb(db);
    char *err_msg = NULL;
    if (sqlite3_exec(outdb.get(), "PRAGMA synchronous=0", NULL, NULL, &err_msg) != SQLITE_OK) {
        std::ostringstream err;
        err << "SQLite Error: Async error: " << err_msg << std::endl;
        throw std::runtime_error(err.str());
    }
    if (sqlite3_exec(outdb.get(), "PRAGMA locking_mode=EXCLUSIVE", NULL, NULL, &err_msg) != SQLITE_OK) {
        std::ostringstream err;
        err << "SQLite Error: Async error: " << err_msg << std::endl;
        throw std::runtime_error(err.str());
    }
    if (sqlite3_exec(outdb.get(), "PRAGMA journal_mode=DELETE", NULL, NULL, &err_msg) != SQLITE_OK) {
        std::ostringstream err;
        err << "SQLite Error: Async error: " << err_msg << std::endl;
        throw std::runtime_error(err.str());
    }
    if (sqlite3_exec(outdb.get(), "CREATE TABLE metadata (name text, value text);", NULL, NULL, &err_msg) != SQLITE_OK) {
        std::ostringstream err;
        err << "SQLite Error: Metadata Table Creation error: " << err_msg << std::endl;
        throw std::runtime_error(err.str());
    }
    if (sqlite3_exec(outdb.get(), "CREATE TABLE tiles (zoom_level integer, tile_column integer, tile_row integer, tile_data blob);", NULL, NULL, &err_msg) != SQLITE_OK) {
        std::ostringstream err;
        err << "SQLite Error: Tiles Table Creation error: " << err_msg << std::endl;
        throw std::runtime_error(err.str());
    }
    if (sqlite3_exec(outdb.get(), "create unique index name on metadata (name);", NULL, NULL, &err_msg) != SQLITE_OK) {
        std::ostringstream err;
        err << "SQLite Error: Metadata Index Creation error: " << err_msg << std::endl;
        throw std::runtime_error(err.str());
    }
    if (sqlite3_exec(outdb.get(), "create unique index tile_index on tiles (zoom_level, tile_column, tile_row);", NULL, NULL, &err_msg) != SQLITE_OK) {
        std::ostringstream err;
        err << "SQLite Error: Tiles Index Creation error: " << err_msg << std::endl;
        throw std::runtime_error(err.str());
    }

    // Construct tile insertion prepared statement
    sqlite3_stmt *stmt;
    const char *query = "insert into tiles (zoom_level, tile_column, tile_row, tile_data) values (?, ?, ?, ?)";
    if (sqlite3_prepare_v2(outdb.get(), query, -1, &stmt, NULL) != SQLITE_OK) {
        std::ostringstream err;
        err << "SQLite Error: Tile prepared statement failed to create." << std::endl;
        throw std::runtime_error(err.str());
    }
    sqlite_stmt_ptr tile_stmt(stmt);
    return { std::move(outdb), std::move(tile_stmt) };
}

void mbtiles_write_tile(sqlite_db const& db, int z, int x, int y, const char *data, int size) {
    sqlite3_stmt *stmt = db.tile_stmt.get();
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    sqlite3_bind_int(stmt, 1, z);
    sqlite3_bind_int(stmt, 2, x);
    sqlite3_bind_int(stmt, 3, (1 << z) - 1 - y);
    sqlite3_bind_blob(stmt, 4, data, size, NULL);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "SQLite Error: tile insert failed: " << sqlite3_errmsg(db.db.get()) << std::endl;
    }
}

inline void quote(std::ostringstream & buf, std::string const& input) {
    for (auto & ch : input) {
        if (ch == '\\' || ch == '\"') {
            buf << '\\';
            buf << ch;
        } else if (ch < ' ') {
            char tmp[7];
            sprintf(tmp, "\\u%04x", ch);
			buf << tmp;
        } else {
			buf << ch;
        }
    }
}

enum json_field_type : std::uint8_t {
    json_field_type_number = 0,
    json_field_type_boolean,
    json_field_type_string
};

struct layer_meta_data {
    int min_zoom;
    int max_zoom;
    std::map<std::string, json_field_type> fields;
};

using layer_map_type = std::map<std::string, layer_meta_data>;

void mbtiles_write_metadata(sqlite_db const& db,
							std::string const& fname, 
							int minzoom,
							int maxzoom,
							layer_map_type const &layermap) {
    char *sql, *err;

    sql = sqlite3_mprintf("INSERT INTO metadata (name, value) VALUES ('name', %Q);", fname.c_str());
    if (sqlite3_exec(db.db.get(), sql, NULL, NULL, &err) != SQLITE_OK) {
        sqlite3_free(sql);
        std::ostringstream err_msg;
        err_msg << "SQLite Error: failed to set name in metadata: " << err << std::endl;
        throw std::runtime_error(err_msg.str());
    }
    sqlite3_free(sql);

    sql = sqlite3_mprintf("INSERT INTO metadata (name, value) VALUES ('description', %Q);", fname.c_str());
    if (sqlite3_exec(db.db.get(), sql, NULL, NULL, &err) != SQLITE_OK) {
        sqlite3_free(sql);
        std::ostringstream err_msg;
        err_msg << "SQLite Error: failed to set description in metadata: " << err << std::endl;
        throw std::runtime_error(err_msg.str());
    }
    sqlite3_free(sql);

    sql = sqlite3_mprintf("INSERT INTO metadata (name, value) VALUES ('version', %d);", 2);
    if (sqlite3_exec(db.db.get(), sql, NULL, NULL, &err) != SQLITE_OK) {
        sqlite3_free(sql);
        std::ostringstream err_msg;
        err_msg << "SQLite Error: failed to set version in metadata: " << err << std::endl;
        throw std::runtime_error(err_msg.str());
    }
    sqlite3_free(sql);

    sql = sqlite3_mprintf("INSERT INTO metadata (name, value) VALUES ('minzoom', %d);", minzoom);
    if (sqlite3_exec(db.db.get(), sql, NULL, NULL, &err) != SQLITE_OK) {
        sqlite3_free(sql);
        std::ostringstream err_msg;
        err_msg << "SQLite Error: failed to set minzoom in metadata: " << err << std::endl;
        throw std::runtime_error(err_msg.str());
    }
    sqlite3_free(sql);

    sql = sqlite3_mprintf("INSERT INTO metadata (name, value) VALUES ('maxzoom', %d);", maxzoom);
    if (sqlite3_exec(db.db.get(), sql, NULL, NULL, &err) != SQLITE_OK) {
        sqlite3_free(sql);
        std::ostringstream err_msg;
        err_msg << "SQLite Error: failed to set maxzoom in metadata: " << err << std::endl;
        throw std::runtime_error(err_msg.str());
    }
    sqlite3_free(sql);
    
    sql = sqlite3_mprintf("INSERT INTO metadata (name, value) VALUES ('center', '0.0,0.0,%d');", maxzoom);
    if (sqlite3_exec(db.db.get(), sql, NULL, NULL, &err) != SQLITE_OK) {
        sqlite3_free(sql);
        std::ostringstream err_msg;
        err_msg << "SQLite Error: failed to set center in metadata: " << err << std::endl;
        throw std::runtime_error(err_msg.str());
    }
    sqlite3_free(sql);
    
    double minlon = -180;
    double minlat = -85.05112877980659;
    double maxlon = 180;
    double maxlat = 85.0511287798066;
    sql = sqlite3_mprintf("INSERT INTO metadata (name, value) VALUES ('bounds', '%f,%f,%f,%f');", minlon, minlat, maxlon, maxlat);
    if (sqlite3_exec(db.db.get(), sql, NULL, NULL, &err) != SQLITE_OK) {
        sqlite3_free(sql);
        std::ostringstream err_msg;
        err_msg << "SQLite Error: failed to set bounds in metadata: " << err << std::endl;
        throw std::runtime_error(err_msg.str());
    }
    sqlite3_free(sql);

    sql = sqlite3_mprintf("INSERT INTO metadata (name, value) VALUES ('type', %Q);", "overlay");
    if (sqlite3_exec(db.db.get(), sql, NULL, NULL, &err) != SQLITE_OK) {
        sqlite3_free(sql);
        std::ostringstream err_msg;
        err_msg << "SQLite Error: failed to set type in metadata: " << err << std::endl;
        throw std::runtime_error(err_msg.str());
    }
    sqlite3_free(sql);

    sql = sqlite3_mprintf("INSERT INTO metadata (name, value) VALUES ('format', %Q);", "pbf");
    if (sqlite3_exec(db.db.get(), sql, NULL, NULL, &err) != SQLITE_OK) {
        sqlite3_free(sql);
        std::ostringstream err_msg;
        err_msg << "SQLite Error: failed to set format in metadata: " << err << std::endl;
        throw std::runtime_error(err_msg.str());
    }
    sqlite3_free(sql);

	std::ostringstream buf;
	buf << "{\"vector_layers\": [ ";  
    
	bool first = true;
	for (auto const& ai : layermap) {
		if (first) {
			first = false;
			buf << "{ \"id\": \"";
		} else {
			buf << ", { \"id\": \"";
		}
		quote(buf, ai.first);
		buf << "\", \"description\": \"\", \"minzoom\": ";
		buf << ai.second.min_zoom;
		buf << ", \"maxzoom\": ";
		buf << ai.second.max_zoom;
        buf << ", \"fields\": {";
		bool first_field = true;
		for (auto const& j : ai.second.fields) {
			if (first_field) {
				first_field = false;
				buf << "\"";
			} else {
				buf << ", \"";
			}
			quote(buf, j.first);
            if (j.second == json_field_type_number) {
                buf << "\": \"Number\"";
            } else if (j.second == json_field_type_boolean) {
                buf << "\": \"Boolean\"";
            } else {
                buf << "\": \"String\"";
            }
		}
		buf << "} }";
    }
	buf << " ] }";

    sql = sqlite3_mprintf("INSERT INTO metadata (name, value) VALUES ('json', %Q);", buf.str().c_str());
    if (sqlite3_exec(db.db.get(), sql, NULL, NULL, &err) != SQLITE_OK) {
        sqlite3_free(sql);
        std::ostringstream err_msg;
        err_msg << "SQLite Error: failed to set json in metadata: " << err << std::endl;
        throw std::runtime_error(err_msg.str());
    }
    sqlite3_free(sql);
}

void mbtiles_close(sqlite_db const& db) {
    char *err;

    if (sqlite3_exec(db.db.get(), "ANALYZE;", NULL, NULL, &err) != SQLITE_OK) {
        std::ostringstream err_msg;
        err_msg << "SQLite Error: analyze failed: " << err << std::endl;
        throw std::runtime_error(err_msg.str());
    }
    // sqlite_db destructor will close the database connection.
}

} // end ns
