import time
import warnings
import requests
import textwrap


def _fetch_data_with_retries(
    url, params=None, headers=None, max_retries=10, wait_time=2
):
    for attempt in range(max_retries):
        try:
            response = requests.get(url, params=params, headers=headers)
            response.raise_for_status()
            return response
        except requests.RequestException as e:
            warnings.warn(f"Attempt {attempt + 1} failed: {e}")
            time.sleep(wait_time)
    raise RuntimeError("All attempts to fetch data failed.")


def get_latest_jupedsim_bibtex(installed_version):
    try:
        response = _fetch_data_with_retries(
            "https://zenodo.org/api/records",
            params={
                "q": "JuPedSim",
                "all_versions": True,
                "sort": "mostrecent",
                "communities": "fire-safety-engineering-and-evacuation",
            },
        )
        records = response.json()["hits"]["hits"]

        if not records:
            raise RuntimeError("No records found for JuPedSim.")

        record_id = None
        for record in records:
            fetched_version = record["metadata"]["version"]

            if fetched_version == installed_version:
                record_id = record["id"]
                break

        if not record_id:
            return textwrap.dedent(f"""\
                No citation information available for this version: {installed_version}
                Please check on Zenodo: https://doi.org/10.5281/zenodo.1293771
                """)

        headers = {"accept": "application/x-bibtex"}
        response = _fetch_data_with_retries(
            f"https://zenodo.org/api/records/{record_id}", headers=headers
        )
        response.encoding = "utf-8"

        if response.status_code == 200:
            return response.text
        else:
            raise RuntimeError("Not found")
    except Exception as e:
        warnings.warn(f"An error occurred: {e}")

# if __name__ == "__main__":
#     jupedsim_bibtex = get_latest_jupedsim_bibtex()
#     print(jupedsim_bibtex)
