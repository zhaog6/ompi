/*
 * $HEADER$
 */

/**
 * @file
 *
 * Setup command line options for the Open MPI Run Time Environment
 */


#include "ompi_config.h"

#include <string.h>

#include "include/constants.h"
#include "util/sys_info.h"
#include "util/proc_info.h"
#include "util/os_path.h"
#include "util/pack.h"
#include "util/session_dir.h"
#include "util/universe_setup_file_io.h"

#include "mca/oob/base/base.h"
#include "mca/ns/base/base.h"

#include "runtime/runtime.h"



int ompi_rte_universe_exists(char *host, char *name, char *tmpdir, char *oob_contact_info)
{
    char *contact_file;
    int32_t command, recv_tag;
    int ret;
    ompi_process_name_t seed={0,0,0};

    /* does universe already exist on specified host? Check session directory to see */
    /* don't know how to handle remote host yet - only cover localhost */

    if (0 != strncmp(host, ompi_system_info.nodename, strlen(ompi_system_info.nodename))) { /* remote host specified */
	fprintf(stderr, "remote hosts not currently supported\n");
	return OMPI_ERR_NOT_IMPLEMENTED;
    }

    /* check to see if local universe already exists */
    if (OMPI_SUCCESS == ompi_session_dir(false, tmpdir, ompi_system_info.user, ompi_system_info.nodename, NULL,
					 name, NULL, NULL)) { /* found */
	/* check for "contact-info" file. if present, read it in. */
	contact_file = ompi_os_path(false, ompi_process_info.universe_session_dir,
				    "universe-setup.txt", NULL);

	if (OMPI_SUCCESS != (ret = ompi_read_universe_setup_file(contact_file))) {
	    return ret;
	}

	if (!ompi_universe_info.persistence ||   /* not persistent... */
	    (0 == strncmp(ompi_universe_info.scope, "local", strlen("local")))) {  /* ...or no connection allowed */
	    return OMPI_ERR_NO_CONNECTION_ALLOWED;
	}

	/* if persistent, set contact info... */
	if (OMPI_SUCCESS != mca_oob_set_contact_info(ompi_universe_info.oob_contact_info)) { /* set contact info */
	    fprintf(stderr, "error setting oob contact info - please report error to bugs@open-mpi.org\n");
	    return OMPI_ERR_FATAL;
	}

	/* 	/\* ...and ping to verify it's alive *\/ */
	/* 	if (OMPI_SUCCESS != mca_oob_ping(&seed)) { */
	/* 	    return OMPI_ERR_CONNECTION_FAILED; */
	/* 	} */

	/* set the my_universe field */
	ompi_process_info.my_universe = strdup(ompi_universe_info.name);
	return OMPI_SUCCESS;
    }
}
