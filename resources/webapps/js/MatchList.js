class MatchList extends Module
{
  constructor (hook)
  {
    super ('MatchList',
           hook);

    this.empty = true;

    {
      let table = document.createElement ('table');

      table.className = 'table_matchlist';
      this.container.appendChild (table);
    }
  }

  clear ()
  {
    super.clear ();
    this.empty = true;
    this.onDirty ();
  }

  isEmpty ()
  {
    return this.empty;
  }

  displayBatch (batch, name)
  {
    batch.display (name, this.container);
    this.empty = false;
    this.onDirty ();
  }
}
