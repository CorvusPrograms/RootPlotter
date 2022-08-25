#include <sol/sol.hpp>
#include "hist_def.h"



void makeBindings(sol::state& lua){
    auto data_source_type =
        lua.new_usertype<DataSource>("DataSource",
                                     sol::constructors<
                                     DataSource(),
                                     DataSource(std::string),
                                     DataSource(std::string, std::string)>(),
                                      "path", &DataSource::tags, 
                                      "name", &DataSource::name,
                                      "tags", &DataSource::tags,
                                      "keys", &DataSource::keys
                                      );
    auto source_set_type =
        lua.new_usertype<SourceSet>("SourceSet",
                                     "name", &SourceSet::name,
                                     "match", &SourceSet::match,
                                     "sources", &SourceSet::sources
                                      );

}




