import subprocess
from pathlib import Path

import pytest

try:
    import ezdxf  # noqa: F401
    import geopandas  # noqa: F401
    import rich  # noqa: F401
    import typer  # noqa: F401
except ImportError:
    pytest.skip("ezdxf module not installed", allow_module_level=True)

dxf_dir = Path("examples/geometry/dxf")
dxf_files = list(dxf_dir.glob("*.dxf"))


@pytest.mark.parametrize("dxf_file", dxf_files)
def test_dxf_conversion(dxf_file):
    """Test that DXF files can be converted to WKT"""
    result = subprocess.run(
        ["python", "scripts/dxf2wkt.py", "convert", "-i", str(dxf_file)],
        capture_output=True,
        text=True,
    )
    assert result.returncode == 0, (
        f"Conversion failed for {dxf_file.name}:\n{result.stderr}"
    )
