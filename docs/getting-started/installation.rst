.. _installation:

Installation
============

Scipp requires Python 3.8 or above.

Conda
-----

The easiest way to install Scipp is using `conda <https://docs.conda.io>`_.
Packages from `Anaconda Cloud <https://conda.anaconda.org/scipp>`_ are available for Linux, macOS, and Windows.
It is recommended to create an environment rather than installing individual packages.

With the provided environment file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Download :download:`scipp.yml <../environments/scipp.yml>` for the latest stable release version of Scipp.
2. In a terminal run:

   .. code-block:: sh

      conda activate
      conda env create -f scipp.yml
      conda activate scipp
      jupyter lab

   The ``conda activate`` ensures that you are in your ``base`` environment.
   This will take a few minutes.
   Above, replace ``scipp.yml`` with the path to the download location you used to download the environment.
   Open the link printed by Jupyter in a browser if it does not open automatically.

If you are new to Scipp, continue reading with `Quick Start <quick-start.rst>`_ and `Data Structures <../user-guide/data-structures.rst>`_.

If you have previously installed Scipp with conda we nevertheless recommend creating a fresh environment rather than trying to ``conda update``.
You may want to remove your old environment first, e.g.,

.. code-block:: sh

   conda activate
   conda env remove -n scipp

and then proceed as per instructions above.
The ``conda activate`` ensures that you are in your ``base`` environment.

Without the provided environment file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To create a new conda environment with Scipp:

.. code-block:: sh

   conda create -n env_with_scipp -c conda-forge -c scipp scipp

To add Scipp to an existing conda environment:

.. code-block:: sh

   conda install -c conda-forge -c scipp scipp

.. note::
   Installing Scipp on Windows requires ``Microsoft Visual Studio 2019 C++ Runtime`` (and versions above) installed.
   Visit https://support.microsoft.com/en-us/topic/the-latest-supported-visual-c-downloads-2647da03-1eea-4433-9aff-95f26a218cc0 for the up to date version of the library.

After installation the module, Scipp can be imported in Python.
Note that only the bare essential dependencies are installed.
If you wish to use plotting functionality you will also need to install ``matplotlib`` and ``ipywidgets``.

To update or remove Scipp use `conda update <https://docs.conda.io/projects/conda/en/latest/commands/update.html>`_ and `conda remove <https://docs.conda.io/projects/conda/en/latest/commands/remove.html>`_.

Pip
---

Scipp is available from `PyPI <https://pypi.org/>`_ via ``pip``:

.. code-block:: sh

   pip install scipp

By default, this is only a minimal install without optional dependencies.
To install components for additional submodules, use the ``all`` extras of the module:

.. code-block:: sh

   pip install scipp[all]

This will install everything that is needed, with the exception of plotting libraries for interactive use in Jupyter.
These are available in the ``interactive`` extra.
We recommend combining this with ``all``:

.. code-block:: sh

   pip install scipp[all,interactive]

From source
-----------

See `developer getting started <../reference/developer/getting-started.rst>`_.
