
#include "gps.hpp"

void *Wncgps::gps_task(void *thread) 
{
    Wncgps        *self = static_cast<Wncgps *>(thread);
    struct timeval gps_start;
    json_keyval    om[12];
    int            k, ok=0;

    char mode_jcmd[]   = "{ \"action\": \"set_loc_mode\", \"args\": { \"mode\": 4 } }";
    char enable_jcmd[] = "{ \"action\": \"set_loc_config\", \"args\": { \"loc\": true } }";

    while( !self->enable_acq ) 
        sleep(1);

    //initialize the M18Qx to perform GPS 
    self->malptr->send_mal_command(mode_jcmd, NULL, 0, false);
    self->malptr->send_mal_command(enable_jcmd, NULL, 0, false);

    //get gps coordinates continuously while gps is on.

    while( self->gps_on ) {
        gettimeofday(&gps_start, NULL);
        time(&self->gps_stat.last_try);
        ok = false;

        while( !ok && self->enable_acq && self->gps_on ) {
            k=self->getGPSlocation(om,sizeof(om));
            while( pthread_mutex_trylock(&self->gps_mutex) )
               pthread_yield();
            for( int i=1; i<k; i++ ) {
                if( !strcmp(om[i].key,"latitude") ) {
                    sscanf( om[i].value, "%f", &self->loc.lat);
                    ok = 1;
                    }
                else if( !strcmp(om[i].key,"longitude") ) {
                    sscanf( om[i].value, "%f", &self->loc.lng);
                    ok = 1;
                    }
                }
            if( ok && self->gps_on ) {
                self->gps_good = true;
                self->gps_stat.last_pos = self->loc;
                time(&self->gps_stat.last_good);
                }
            else{
                self->gps_good = false;
                }
            pthread_mutex_unlock(&self->gps_mutex);
            sleep(1);
            }
        } 
    pthread_exit(0);
}

int Wncgps::getGPSlocation(json_keyval *kv, int kvsize) 
{
    char rstr[300];
    char jcmd[] = "{ \"action\" : \"get_loc_position_info\" }";

    memset(rstr,0x00,sizeof(rstr));
    if( !malptr->send_mal_command(jcmd, rstr, sizeof(rstr), true) ) 
        return malptr->parse_maljson (rstr, kv, kvsize);

    return 0;
}

