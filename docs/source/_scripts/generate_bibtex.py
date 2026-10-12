"""Fetch the citation information for this build from Zenodo.

The record is looked up by *concept id*, the identifier that stays the same
across all releases, rather than by searching for the project name. A name
search matches titles, descriptions and author names, so it can return an
unrelated record, and it reads only one page of results. Zenodo caps that page
at 25 records for anonymous requests, and JuPedSim has more published versions
than that, so versions below the cap were no longer findable at all.

Zenodo being briefly unreachable must not fail the documentation build, so this
always returns a string: the record for the version being built, otherwise the
most recent release, otherwise a comment pointing at the DOI.
"""

import logging

from zenodo_bibtex_exporter import ZenodoBibtexError, get_bibtex

logger = logging.getLogger(__name__)

#: JuPedSim on Zenodo. This is the concept id, which never changes between releases.
CONCEPT_ID = "1293771"

CONCEPT_DOI_URL = f"https://doi.org/10.5281/zenodo.{CONCEPT_ID}"


def get_latest_jupedsim_bibtex(installed_version: str) -> str:
    """Return the BibTeX entry to publish with the documentation.

    Args:
        installed_version: Version being documented, as published on Zenodo,
            for example ``v1.4.2``. Matched verbatim.

    Returns:
        A BibTeX entry, or a BibTeX comment explaining why there is none.
    """
    try:
        entry = get_bibtex(CONCEPT_ID, version=installed_version)
    except ZenodoBibtexError as error:
        logger.warning("No Zenodo record for %s: %s", installed_version, error)
    else:
        logger.info("Using the Zenodo record for %s.", installed_version)
        return entry

    try:
        entry = get_bibtex(CONCEPT_ID)
    except ZenodoBibtexError as error:
        logger.warning("No citation information available at all: %s", error)
        return (
            f"% Citation information could not be retrieved from Zenodo.\n"
            f"% It is available at {CONCEPT_DOI_URL}\n"
        )
    else:
        logger.info("Falling back to the most recent release on Zenodo.")
        return entry


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO, format="%(levelname)s: %(message)s")
    print(get_latest_jupedsim_bibtex("v1.4.2"), end="")
